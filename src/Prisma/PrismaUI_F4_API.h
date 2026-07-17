/*
 * For modders: Copy this file into your own project if you wish to use this API.
 */
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <Windows.h>
#include <stdint.h>

typedef uint64_t PrismaView;

namespace PRISMA_UI_API {
    constexpr const auto PrismaUIPluginName = "PrismaUI_F4";

    // Available PrismaUI interface versions
    enum class InterfaceVersion : uint8_t { V1, V2, V3, V4, V5, V6, V7, V8 };

    typedef void (*OnDomReadyCallback)(PrismaView view);
    typedef void (*JSCallback)(const char* result);
    typedef void (*JSListenerCallback)(const char* argument);

    // JavaScript console message severity level for use with RegisterConsoleCallback().
    enum class ConsoleMessageLevel : uint8_t { Log = 0, Warning, Error, Debug, Info };

    // Console message callback.
    typedef void (*ConsoleMessageCallback)(PrismaView view, ConsoleMessageLevel level, const char* message);

    // PrismaUI modder interface v1
    class IVPrismaUI1 {
    protected:
        ~IVPrismaUI1() = default;

    public:
        // Create view.
        virtual PrismaView CreateView(const char* htmlPath,
                                      OnDomReadyCallback onDomReadyCallback = nullptr) noexcept = 0;

        // Send JS code to UI.
        virtual void Invoke(PrismaView view, const char* script, JSCallback callback = nullptr) noexcept = 0;

        // Call JS function through JS Interop API (best performance).
        virtual void InteropCall(PrismaView view, const char* functionName, const char* argument) noexcept = 0;

        // Register JS listener.
        virtual void RegisterJSListener(PrismaView view, const char* functionName,
                                        JSListenerCallback callback) noexcept = 0;

        // Returns true if view has focus.
        virtual bool HasFocus(PrismaView view) noexcept = 0;

        // Set focus on view.
        virtual bool Focus(PrismaView view, bool pauseGame = false, bool disableFocusMenu = false) noexcept = 0;

        // Remove focus from view.
        virtual void Unfocus(PrismaView view) noexcept = 0;

        // Show a hidden view.
        virtual void Show(PrismaView view) noexcept = 0;

        // Hide a visible view.
        virtual void Hide(PrismaView view) noexcept = 0;

        // Returns true if view is hidden.
        virtual bool IsHidden(PrismaView view) noexcept = 0;

        // Get scroll size in pixels.
        virtual int GetScrollingPixelSize(PrismaView view) noexcept = 0;

        // Set scroll size in pixels.
        virtual void SetScrollingPixelSize(PrismaView view, int pixelSize) noexcept = 0;

        // Returns true if view exists.
        virtual bool IsValid(PrismaView view) noexcept = 0;

        // Completely destroy view.
        virtual void Destroy(PrismaView view) noexcept = 0;

        // Set view order.
        virtual void SetOrder(PrismaView view, int order) noexcept = 0;

        // Get view order.
        virtual int GetOrder(PrismaView view) noexcept = 0;

        // Create inspector view for debugging.
        virtual void CreateInspectorView(PrismaView view) noexcept = 0;

        // Show or hide the inspector overlay.
        virtual void SetInspectorVisibility(PrismaView view, bool visible) noexcept = 0;

        // Returns true if inspector is visible.
        virtual bool IsInspectorVisible(PrismaView view) noexcept = 0;

        // Set inspector window position and size.
        virtual void SetInspectorBounds(PrismaView view, float topLeftX, float topLeftY, unsigned int width,
                                        unsigned int height) noexcept = 0;

        // Returns true if any view has active focus.
        virtual bool HasAnyActiveFocus() noexcept = 0;
    };

    // PrismaUI modder interface v2 (extends v1)
    class IVPrismaUI2 : public IVPrismaUI1 {
    protected:
        ~IVPrismaUI2() = default;

    public:
        // Register a callback to receive JavaScript console messages from a view.
        // Pass nullptr to unregister.
        virtual void RegisterConsoleCallback(PrismaView view, ConsoleMessageCallback callback) noexcept = 0;
    };

    // PrismaUI modder interface v3 (extends v2)
    class IVPrismaUI3 : public IVPrismaUI2 {
    protected:
        ~IVPrismaUI3() = default;

    public:
        // Load Data\Interface\Translations\<pluginName>_<lang>.txt for the game's language and
        // inject window.L10N and window.t() into the page. Call right after CreateView.
        // See docs/translations.md.
        virtual void RegisterTranslations(PrismaView view, const char* pluginName) noexcept = 0;
    };

    // Callback type for EnumerateViews. Called once per view with the view's ID and its
    // original html path (relative to Data/PrismaUI_F4/views/, e.g. "debug_panel.html").
    typedef void (*ViewEnumCallback)(PrismaView id, const char* htmlPath, void* userdata);

    // PrismaUI modder interface v4 (extends v3)
    class IVPrismaUI4 : public IVPrismaUI3 {
    protected:
        ~IVPrismaUI4() = default;

    public:
        // JS listener whose callback runs on the game thread, so you can touch RE:: state
        // directly without AddTask. Prefer this over RegisterJSListener when you need game state.
        virtual void BindUIEvent(PrismaView view, const char* functionName,
                                 JSListenerCallback callback) noexcept = 0;

        // Enumerate all currently-registered views across all plugins.
        // Callback is invoked synchronously for each view; htmlPath is the relative path
        // passed to CreateView (e.g., "Interface/PrismaMCM/mcm.html").
        // Safe to call from any thread.
        virtual void EnumerateViews(ViewEnumCallback callback, void* userdata) noexcept = 0;
    };

    // PrismaUI modder interface v5 (extends v4)
    class IVPrismaUI5 : public IVPrismaUI4 {
    protected:
        ~IVPrismaUI5() = default;

    public:
        // The view's live D3D11 SRV (ID3D11ShaderResourceView*), as void* to keep d3d11.h out of
        // this header. Null until the view has rendered a frame. Don't Release() it.
        virtual void* GetViewSRV(PrismaView view) noexcept = 0;

        // Keep rendering the view to its texture but never draw it as a 2D overlay.
        // Use for on-mesh rendering.
        virtual void SetViewOffscreen(PrismaView view, bool offscreen) noexcept = 0;

        // Draw the view onto a 3D surface. rootObject is an RE::NiAVObject* (void* keeps
        // CommonLib out of this header); geometryName is a node under it, e.g. "Screen:0".
        // Call SetViewOffscreen(true) first, bind once the geometry exists, and re-bind on each
        // open since the model can be rebuilt. See docs/on-mesh-rendering.md.
        virtual bool BindViewToGeometry(PrismaView view, void* rootObject, const char* geometryName) noexcept = 0;

        // Same, but finds the target by diffuse texture path instead of node name. Useful for the
        // Pip-Boy screen, which every replacer maps to "PipBoyScreen_d".
        virtual bool BindViewToScreenTexture(PrismaView view, void* rootObject, const char* textureSubstring) noexcept = 0;

        // Restore the geometry's original texture. Call while it is still in the scene.
        virtual void UnbindViewFromGeometry(PrismaView view) noexcept = 0;
    };

    // PrismaUI modder interface v6 (extends v5)
    class IVPrismaUI6 : public IVPrismaUI5 {
    protected:
        ~IVPrismaUI6() = default;

    public:
        // Hide or restore a vanilla HUD widget, e.g. "HUDCompass", "HUDAmmoCounter".
        // Survives HUD rebuilds and save loads. False on an unknown name, or on NG/AE.
        virtual bool SuppressHUDWidget(const char* className, bool suppress) noexcept = 0;

        // Hide a vanilla menu by MENU_NAME, e.g. "PipboyMenu". Reapplied on every reopen.
        virtual bool SuppressVanillaMenu(const char* menuName, bool suppress) noexcept = 0;

        // Close a vanilla menu now. Pair with SuppressVanillaMenu to keep it closed.
        virtual bool CloseVanillaMenu(const char* menuName) noexcept = 0;
    };

    // Return true to suppress this open, false to allow it. Runs on the game thread, so keep it
    // cheap.
    typedef bool (*MenuSuppressPredicate)();

    // PrismaUI modder interface v7 (extends v6)
    class IVPrismaUI7 : public IVPrismaUI6 {
    protected:
        ~IVPrismaUI7() = default;

    public:
        // SuppressVanillaMenu, but the predicate decides on each open. Pass null to unregister.
        virtual void SuppressVanillaMenuIf(const char* menuName, MenuSuppressPredicate predicate) noexcept = 0;

        // Filter the vanilla "E) TAKE  R) TRANSFER" row on lootable refs. OG only.
        // dropDefaultTake also removes the default Take button. Doors, terminals and NPCs are left
        // alone. The first run logs each choice's perk FormID so you can find what to drop.
        virtual void EnableActivateChoiceFilter(bool enable, bool dropDefaultTake) noexcept = 0;

        // Drop or restore one perk's entry in that row, by runtime FormID.
        virtual void SuppressActivateChoicePerk(uint32_t perkFormID, bool suppress) noexcept = 0;
    };

    // Like ViewEnumCallback, plus the module that created the view, e.g. "MyPlugin.dll".
    // Empty if it could not be resolved.
    typedef void (*ViewEnumCallbackEx)(PrismaView id, const char* htmlPath, const char* owner, void* userdata);

    // Health of a view, from GetViewHealth.
    enum class ViewHealth : int {
        kUnknown = -1,         // CEF not active, or `view` is not a currently-known handle
        kCreating = 0,         // CreateView issued, iframe mounting
        kDomReady = 1,         // OnDomReady fired
        kLive = 2,             // healthy / interactive
        kLoadFailed = 3,       // page/iframe failed to load
        kDomReadyTimeout = 4,  // never fired OnDomReady within the watchdog window
        kUnresponsive = 5,     // missed liveness pings
        kJsError = 6,          // accumulating uncaught/console errors (not fatal; flagged)
    };

    // PrismaUI modder interface v8 (extends v7)
    class IVPrismaUI8 : public IVPrismaUI7 {
    protected:
        ~IVPrismaUI8() = default;

    public:
        // Same as EnumerateViews (V4), but also reports which plugin created each view. Callback is
        // invoked synchronously for each view; safe to call from any thread.
        virtual void EnumerateViewsEx(ViewEnumCallbackEx callback, void* userdata) noexcept = 0;

        // --- appended in-place to V8 (ABI-safe: added at the END of the vtable, existing offsets
        // Read the label of the captured activate choice at buttonIndex (0..3), e.g. "Field Dress".
        // Needs EnableActivateChoiceFilter(true, ...). False if that slot is empty.
        virtual bool GetActivateChoiceLabel(uint32_t buttonIndex, char* outBuffer, size_t bufferSize) noexcept = 0;

        // Fire the captured choice at buttonIndex, replaying the same call the engine makes when
        // the vanilla row's button is clicked. Call it soon after reading the label with
        // GetActivateChoiceLabel: the captured listener isn't reference-counted here, so it can go
        // stale if the player's target changes in between. Returns false if that slot is empty.
        virtual bool TriggerActivateChoice(uint32_t buttonIndex) noexcept = 0;

        // Current health of a view: load errors, piling-up JS console errors, a missed onDomReady,
        // missed liveness pings. Returns kUnknown if CEF isn't running or the handle is unknown.
        // See ViewHealth below for the states.
        virtual ViewHealth GetViewHealth(PrismaView view) noexcept = 0;

        // Resolution of a view's offscreen browser. Match the aspect ratio of the target quad, not
        // the game window, or the page comes out stretched. Safe to call before SetViewOffscreen.
        // See docs/on-mesh-rendering.md.
        virtual void SetViewOffscreenSize(PrismaView view, int width, int height) noexcept = 0;
    };

    // Maps an interface type to its version, so you can only ask for one that exists.
    template <typename T>
    struct InterfaceVersionMap;

    template <>
    struct InterfaceVersionMap<IVPrismaUI1> {
        static constexpr InterfaceVersion version = InterfaceVersion::V1;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI2> {
        static constexpr InterfaceVersion version = InterfaceVersion::V2;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI3> {
        static constexpr InterfaceVersion version = InterfaceVersion::V3;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI4> {
        static constexpr InterfaceVersion version = InterfaceVersion::V4;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI5> {
        static constexpr InterfaceVersion version = InterfaceVersion::V5;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI6> {
        static constexpr InterfaceVersion version = InterfaceVersion::V6;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI7> {
        static constexpr InterfaceVersion version = InterfaceVersion::V7;
    };

    template <>
    struct InterfaceVersionMap<IVPrismaUI8> {
        static constexpr InterfaceVersion version = InterfaceVersion::V8;
    };

    typedef void* (*RequestPluginAPIFunc)(InterfaceVersion interfaceVersion);

    [[nodiscard]] inline void* RequestPluginAPI(InterfaceVersion a_interfaceVersion = InterfaceVersion::V1) {
        auto pluginHandle = GetModuleHandleW(L"PrismaUI_F4.dll");
        if (!pluginHandle) {
            return nullptr;
        }

        auto requestAPIFunction =
            reinterpret_cast<RequestPluginAPIFunc>(GetProcAddress(pluginHandle, "RequestPluginAPI"));

        if (requestAPIFunction) {
            return requestAPIFunction(a_interfaceVersion);
        }

        return nullptr;
    }

    template <typename T>
    [[nodiscard]] inline T* RequestPluginAPI() {
        return static_cast<T*>(RequestPluginAPI(InterfaceVersionMap<T>::version));
    }
}
