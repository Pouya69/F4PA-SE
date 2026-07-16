#include "Highwayman_Map.h"
#include "../Shared.h"
#include <REX/LOG.h>
#include "PrismaUI_F4_API.h"
#include <cassert>
#include <format>
#include <string>
#include <RE/B/BGSLocation.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESWorldSpace.h>
#include "../IGlobalConfig.h"

namespace PArroyo {
	namespace Highwayman {
		PrismaView                   g_view = 0;
		bool                         g_visible = false;

		RE::BGSSoundDescriptorForm* Highwayman_SongDescriptorRef;
		RE::BGSSoundDescriptorForm* Highwayman_EngineStartDescriptorRef;
		RE::BGSSoundDescriptorForm* Highwayman_LoopDescriptorRef;
		RE::BGSSoundDescriptorForm* Highwayman_LoopAndStartDescriptorRef;

		RE::BSSoundHandle* currentLoopHandle;
		RE::BSSoundHandle* currentStartupHandle;
		RE::BSSoundHandle* currentSongHandle;

		std::uint32_t SongHandleID = -1, EngineLoopID = -1, StartupID = -1;

		void OnDomReady(PrismaView view) {
			REX::DEBUG("Highwayman DOM loaded!");
			Shared::prisma_api->BindUIEvent(g_view, "PlayerTravelToLocation", PlayerTravelToLocation);
			Shared::prisma_api->BindUIEvent(g_view, "PlayerExitMap", PlayerExitMap);
			
		}

		void InitializePrismaType(bool bHide) {
			if (Shared::prisma_api->IsValid(g_view))
				return;

			g_view = Shared::prisma_api->CreateView("Highwayman/index.html", OnDomReady);
			if (bHide) {
				g_visible = false;
				Shared::prisma_api->Hide(g_view);
			}
			// Shared::prisma_api->CreateInspectorView(g_view);

			

			Shared::prisma_api->RegisterConsoleCallback(g_view,
				[](PrismaView, PRISMA_UI_API::ConsoleMessageLevel, const char* msg) {
					REX::INFO("[JS] {}", msg);
				});

			REX::DEBUG("Initialized Highwayman Prisma.");
			
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
			
			if (currentLoopHandle) {
				
				currentLoopHandle->Stop();
			}
			if (currentStartupHandle) {
				currentStartupHandle->Stop();
			}
			if (currentSongHandle) {
				currentSongHandle->Stop();
			}
			
			RE::BSSoundHandle handle, handleLoop, songHandle;

			RE::BSAudioManager::GetSingleton()->GetSoundHandle(handle, Highwayman_EngineStartDescriptorRef, 0.0f, 0);
			RE::BSAudioManager::GetSingleton()->GetSoundHandle(handleLoop, Highwayman_LoopDescriptorRef, 0.0f, 0);
			RE::BSAudioManager::GetSingleton()->GetSoundHandle(songHandle, Highwayman_SongDescriptorRef, 0.0f, 0);
			handle.Stop();
			handleLoop.Stop();
			songHandle.Stop();

			StartupID = handle.soundID;
			EngineLoopID = handleLoop.soundID;
			SongHandleID = songHandle.soundID;

			handle.Play();
			handleLoop.Play();
			songHandle.Play();

			currentLoopHandle = &handleLoop;
			currentStartupHandle = &handle;
			currentSongHandle = &songHandle;

			Shared::prisma_api->Show(g_view);
			g_visible = true;

			Shared::prisma_api->Focus(g_view, true);

			const auto currentPlayerLocation = RE::PlayerCharacter::GetSingleton()->GetCurrentLocation();

			Shared::prisma_api->InteropCall(g_view, "LoadHighwayman", std::format("\"{}\"", GetDestinationNameForJS(currentPlayerLocation)).c_str());
		}

		void PlayerTravelToLocation(const char* arg)
		{
			REX::DEBUG("Player travel in Highwayman requested... arg: {}", arg);
			/*
			if (!Shared::HighwaymanWorldspacesMap.contains(arg)) {
				REX::WARN("Argument is not a worldspace in the hashmap for Highwayman... arg: {}", arg);
				return;
			}

			auto result = Shared::HighwaymanWorldspacesMap.at(arg);
			*/
			auto player = RE::PlayerCharacter::GetSingleton();

			REX::DEBUG("Travelling to {}...", arg);

			//auto loc = Shared::HighwaymanLocationsMap.at("00PA_ArroyoLocation");

			// KlamathBath for test.
			auto loc = static_cast<RE::BGSLocation*>(Shared::LocationsMapHighwaymanList->arrayOfForms.at(9));

			//auto marker = loc->worldLocMarker;
			auto marker = player->GetCurrentLocation()->worldLocMarker;
			//REX::DEBUG("Travel to '{}' for testing... World marker valid status: {}", loc->GetFullName(), loc->worldLocMarker.get() != nullptr ? 1 : 0);

			auto vm = RE::GameVM::GetSingleton()->GetVM();

			vm->DispatchStaticCall("Game", "FastTravel", nullptr, marker.get().get());
			// player->QueueFastTravel(player->GetCurrentLocation()->worldLocMarker.get().get(), false);
			
			//player->QueueFastTravel(result->location->horseLocMarker ? result->location->horseLocMarker : result->location->worldLocMarker, true);
			
			
		}

		void PlayerExitMap(const char* arg)
		{
			CloseHighwayman();
		}

		void CloseHighwayman() {
			g_visible = false;
			Shared::prisma_api->Unfocus(g_view);
			Shared::prisma_api->Hide(g_view);

			if (currentLoopHandle) {

				currentLoopHandle->Stop();
			}
			if (currentStartupHandle) {
				currentStartupHandle->Stop();
			}
			if (currentSongHandle) {
				currentSongHandle->Stop();
			}

			RE::BSSoundHandle handle, handleLoop, songHandle;

			RE::BSAudioManager::GetSingleton()->GetSoundHandle(handle, Highwayman_EngineStartDescriptorRef, 0.0f, 0);
			RE::BSAudioManager::GetSingleton()->GetSoundHandle(handleLoop, Highwayman_LoopDescriptorRef, 0.0f, 0);
			RE::BSAudioManager::GetSingleton()->GetSoundHandle(songHandle, Highwayman_SongDescriptorRef, 0.0f, 0);

			handle.soundID = StartupID;
			handleLoop.soundID = EngineLoopID;
			songHandle.soundID = SongHandleID;

			handle.Stop();
			handleLoop.Stop();
			songHandle.Stop();

			RE::BSSoundHandle a;
			a.Stop();

		}

		std::string GetDestinationNameForJS(const RE::BGSLocation* location)
		{
			return location ? location->GetFullName() : "";
		}

		void InitializeHighwaymanMapLocations()
		{
			auto dataHandler = RE::TESDataHandler::GetSingleton();

			Shared::HighwaymanWorldspacesMap.emplace("00PAArroyo", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PAArroyo"));
			Shared::HighwaymanWorldspacesMap.emplace("00PAHomiganFight", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PAHomiganFight"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANCFCenter", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANCFCenter"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANCRSouth", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANCRSouth"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANCRWest", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANCRWest"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANewReno", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANewReno"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANewRenoEastChop", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANewRenoEastChop"));
			Shared::HighwaymanWorldspacesMap.emplace("00PANewRenoWest", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PANewRenoWest"));
			Shared::HighwaymanWorldspacesMap.emplace("00PARegion1Klamath", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PARegion1Klamath"));
			Shared::HighwaymanWorldspacesMap.emplace("00PARegion2ModocForest", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PARegion2ModocForest"));
			Shared::HighwaymanWorldspacesMap.emplace("00PARegion3Gecko", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PARegion3Gecko"));
			Shared::HighwaymanWorldspacesMap.emplace("00PARegion5Reno", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PARegion5Reno"));
			Shared::HighwaymanWorldspacesMap.emplace("00PARegion7Mariposa", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PARegion7Mariposa"));
			Shared::HighwaymanWorldspacesMap.emplace("00PASanFranRegion1", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PASanFranRegion1"));
			Shared::HighwaymanWorldspacesMap.emplace("00PASierraArmyDepotSecretPlace2", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PASierraArmyDepotSecretPlace2"));
			Shared::HighwaymanWorldspacesMap.emplace("00PASierraArmyDepotSecretPlacee", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PASierraArmyDepotSecretPlacee"));
			Shared::HighwaymanWorldspacesMap.emplace("00PAVault13", RE::TESWorldSpace::GetFormByEditorID<RE::TESWorldSpace>("00PAVault13"));

			assert(Shared::HighwaymanWorldspacesMap.at("00PAArroyo") != nullptr);

			Shared::LocationsMapHighwaymanList = dataHandler->LookupForm<RE::BGSListForm>(0x006788, CURRENT_ESP);

			for (auto a : Shared::LocationsMapHighwaymanList->arrayOfForms) {
				RE::BGSLocation* location = static_cast<RE::BGSLocation*>(a);
				Shared::HighwaymanLocationsMap.emplace(location->GetFormEditorID(), location);
			}

			Highwayman_SongDescriptorRef = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x00971C, CURRENT_ESP);
			Highwayman_LoopDescriptorRef = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x00A0A1, CURRENT_ESP);
			Highwayman_LoopAndStartDescriptorRef = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x00A0A2, CURRENT_ESP);
			Highwayman_EngineStartDescriptorRef = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x00A0A0, CURRENT_ESP);

		}

		void HighwaymanMapFullyLoaded(const char* arg)
		{
			
		}
	}
}
