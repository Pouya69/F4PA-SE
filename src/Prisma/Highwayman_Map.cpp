#include "Highwayman_Map.h"
#include "../Shared.h"
#include <REX/LOG.h>
#include "PrismaUI_F4_API.h"

namespace PArroyo {
	namespace Highwayman {
		PrismaView                   g_view = 0;
		bool                         g_visible = false;

		void OnDomReady(PrismaView view) {
			REX::DEBUG("Highwayman DOM loaded!");
			Shared::prisma_api->BindUIEvent(g_view, "PlayerTravelToLocation", PlayerTravelToLocation);

		}

		void InitializePrismaType(bool bHide) {
			g_view = Shared::prisma_api->CreateView("Highwayman/index.html", OnDomReady);
			if (bHide) {
				g_visible = false;
				Shared::prisma_api->Hide(g_view);
			}

			Shared::prisma_api->RegisterConsoleCallback(g_view,
				[](PrismaView, PRISMA_UI_API::ConsoleMessageLevel, const char* msg) {
					REX::INFO("[JS] {}", msg);
				});
			
		}

		void CheckIfNotValidAndCreate()
		{
			if (!Shared::prisma_api->IsValid(g_view)) {
				InitializePrismaType(false);
			}
		}

		void OpenUpHighwaymanMap()
		{
			CheckIfNotValidAndCreate();

			Shared::prisma_api->Show(g_view);
			g_visible = true;
		}

		void PlayerTravelToLocation(const char* arg)
		{
			
		}

		void PlayerExitMap(const char* arg)
		{
			g_visible = false;
			Shared::prisma_api->Hide(g_view);
		}

		void HighwaymanMapFullyLoaded(const char* arg)
		{
			
		}
	}
}
