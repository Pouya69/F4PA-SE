#include "main.h"

#include <F4SE/API.h>
#include <F4SE/Interfaces.h>
#include <RE/B/BSScript_IVirtualMachine.h>

#include "debugLog.h"
#include "skills.h"
#include <RE/T/TESDataHandler.h>
#include "IGlobalConfig.h"
#include "ObScript.h"
#include "Hooks.h"
#include "ItemDegradation.h"
#include "Menus/HUD_Additions.h"
#include "Menus/LevelUpMenu.h"
#include "Menus/PipboyTabs.h"
#include "Menus/RepairMenu.h"
#include "Menus/Workbench_Additions.h"
#include "Events/OnHitEvent.h"
#include "Events/OnEquipEvent.h"
#include "Events/OnMenuOpenCloseEvent.h"
#include "Events/OnLevelUpEvent.h"
#include "Prisma/PrismaUI_F4_API.h"
#include <format>
#include <string_view>
#include <variant>
#include <REX/LOG.h>
#include "Shared.h"
#include "Prisma/Highwayman_Map.h"


void InitializeGameEvents()
{
	F4CWEvents::RegisterOnMenuOpenCloseEvent();
	F4CWEvents::RegisterOnEquipEvent();
	F4CWEvents::RegisterOnHitEvent();
	F4CWEvents::RegisterOnLevelUpEvent();
}

bool RegisterCustomFunctions(RE::BSScript::IVirtualMachine* vm) {
	vm->BindNativeMethod("PArroyo", "PouyaFunction", PArroyo::PouyaFunction, false, false);

	if (!RegisterSkillFunctions(*vm)) {
		REX::WARN("Could not register Karma funcitons...");
		return false;
	}

	return true;
}

bool RegisterScaleforms(const F4SE::ScaleformInterface* scaleformInterface) {
	if (!scaleformInterface->Register("CWHUDAdditions", PArroyo_Menus::HUD_Additions::RegisterScaleform)) {
		REX::CRITICAL("Failed to register 'CWHUDAdditions' Scaleform."sv);
		return false;
	}
	REX::DEBUG("Registered CWHUDAdditions");

	PArroyo_Menus::LevelUpMenu::RegisterMenu();
	if (!scaleformInterface->Register("CWLevelUpMenu", PArroyo_Menus::LevelUpMenu::RegisterScaleform))
	{
		REX::CRITICAL("Failed to register 'LevelUpMenu', marking as incompatible."sv);
		return false;
	}
	REX::DEBUG("Registered CWLevelUpMenu");

	if (!scaleformInterface->Register("CWPipboy", PArroyo_Menus::PipboyTabs::RegisterScaleform)) {
		REX::CRITICAL("Failed to register 'CWPipboy', marking as incompatible"sv);
		return false;
	}
	REX::DEBUG("Registered CWPipboy");

	PArroyo_Menus::RepairMenu::CreateRepairMenu();
	REX::DEBUG("Registered CWRepairMenu");
	if (!scaleformInterface->Register("CWRepairMenu", PArroyo_Menus::RepairMenu::RegisterScaleform)) {
		REX::CRITICAL("Failed to register 'CWRepairMenu', marking as incompatible"sv);
		return false;
	}
	REX::DEBUG("Registered CWRepairMenu");

	if (!scaleformInterface->Register("PAWorkbenchAdditions", PArroyo_Menus::Workbench_Additions::RegisterScaleform)) {
		REX::CRITICAL("Failed to register 'PAWorkbenchAdditions' Scaleform."sv);
		return false;
	}
	REX::DEBUG("Registered PAWorkbenchAdditions");

	return true;
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{

	F4SE::Init(a_f4se);
	LOG_INFO("I am Pouya this is my first Fallout 4 CommonLib Mod.");
	
	LOG_WARNING(std::format("Pouya: F4SE Version: {}. Expected version: {}", F4SE::GetF4SEVersion(), F4SE_VERSION));
	
	if (F4SE::GetF4SEVersion() != F4SE_VERSION) {
		REX::DEBUG("F4SE Versions are not matching.");

		return false;
	}

	LOG_INFO("Installing Hooks...");
	PArroyo::Hooks::Registers::Install();
	LOG_INFO("Hooks installed");

	// LOG_TO_CONSOLE("This is a console log by Pouya");

	// RE::TESDataHandler::GetSingleton()->LookupForm();

	if (!F4SE::GetPapyrusInterface()->Register(RegisterCustomFunctions)) {
		REX::WARN("Functions could not be registered.");
		return false;
	}

	F4SE::GetMessagingInterface()->RegisterListener(F4SEMessageCallback);

	const F4SE::ScaleformInterface* scaleformInterface = F4SE::GetScaleformInterface();
	if (!scaleformInterface) {
		REX::CRITICAL("Failed to fetch Scaleform Interface."sv);
		return false;
	}

	if (!RegisterScaleforms(scaleformInterface)) {
		REX::CRITICAL("Failed to register Scaleform custom Menus."sv);
		return false;
	}

	LOG_INFO("Registering hooks...");
	PArroyo::Hooks::Registers::RegisterAllHooks();
	LOG_INFO("All hooks are registered.");

	return true;
}

void PArroyo::PouyaFunction(std::monostate)
{
	LOG_TO_CONSOLE("FIRST FUNCTION OF POUYA THAT RUNS ON ACTORS ONLY!");
}

bool InitializeSharedGameVariables()
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler) {
		REX::WARN("Initializing game variables failed... Data Handler is null");
		return false;
	}


	InitializeGameVariables(dataHandler, MOD_ESM);

	Shared::InitializeSharedForms(dataHandler);
	LOG_INFO("All variables initialized.");




	return true;
}

static void OnDomReady(PrismaView /*view*/)
{
	REX::INFO("MyPlugin: DOM ready");
}

void InitializePrisma() {
	Shared::prisma_api = PRISMA_UI_API::RequestPluginAPI<PRISMA_UI_API::IVPrismaUI4>();
	if (!Shared::prisma_api) {
		REX::WARN("PrismaUI V4 Not Found");
		return;
	}

	REX::DEBUG("PrismaUI V4 Is there.");
}

void GameDataLoaded()
{

}

void InitializeCustomConsoleCommands() {
	PArroyo::ObScript::Install();
}

void GameDataReady()
{
	if (InitializeSharedGameVariables()) {
		Skills::RegisterForSkillLink();
	}
	else {
		REX::WARN("Initializing Shared Game Variables failed.");
	}

	InitializeCustomConsoleCommands();
	LOG_INFO("All console commands initilaized.");

	PArroyo::ItemDegradation::DefineItemDegradationFormsFromGame();

	InitializeGameEvents();

	InitializePrisma();

	LOG_INFO("PACW Plugin is fully ready.");
}

static void InitializePrismaTypes() {
	if (!Shared::prisma_api)
		return;

	PArroyo::Highwayman::InitializePrismaType();
}

void NewGame() {
	InitializePrismaTypes();
}

void PostLoadGame() {
	InitializePrismaTypes();
}

void F4SEMessageCallback(F4SE::MessagingInterface::Message* myMessage) {
	switch (myMessage->type)
	{
		case F4SE::MessagingInterface::kGameDataReady:
			GameDataReady();
			break;

		case F4SE::MessagingInterface::kPostLoadGame:
			PostLoadGame();
			break;

		case F4SE::MessagingInterface::kNewGame:
			NewGame();
			break;

		case F4SE::MessagingInterface::kGameLoaded:
			GameDataLoaded();
			break;
		default:
			break;
	}
}

