#pragma once
#include "PrismaUI_F4_API.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <string>
#include <RE/B/BGSLocation.h>

namespace PArroyo {
	namespace Highwayman {
		extern PrismaView                   g_view;
		extern bool                         g_visible;

		extern RE::BGSSoundDescriptorForm* Highwayman_SongDescriptorRef;
		extern RE::BGSSoundDescriptorForm* Highwayman_EngineStartDescriptorRef;
		extern RE::BGSSoundDescriptorForm* Highwayman_LoopDescriptorRef;
		extern RE::BGSSoundDescriptorForm* Highwayman_LoopAndStartDescriptorRef;

		extern RE::BSSoundHandle* currentLoopHandle;
		extern RE::BSSoundHandle* currentStartupHandle;
		extern RE::BSSoundHandle* currentSongHandle;

		extern std::uint32_t SongHandleID, EngineLoopID, StartupID;

		void OnDomReady(PrismaView view);

		void InitializePrismaType(bool bHide = true);

		void CheckIfNotValidAndCreate();

		void OpenUpHighwaymanMap();

		void PlayerTravelToLocation(const char* arg);

		void PlayerExitMap(const char* arg);

		void HighwaymanMapFullyLoaded(const char* arg);
		
		void CloseHighwayman();

		std::string GetDestinationNameForJS(const RE::BGSLocation* location);

		void InitializeHighwaymanMapLocations();
	}
}