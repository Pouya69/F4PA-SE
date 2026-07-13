#pragma once
#include "PrismaUI_F4_API.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace PArroyo {
	namespace Highwayman {
		extern PrismaView                   g_view;
		extern bool                         g_visible;

		static void OnDomReady(PrismaView view);

		void InitializePrismaType(bool bHide = true);

		static void CheckIfNotValidAndCreate();

		static void OpenUpHighwaymanMap();

		static void PlayerTravelToLocation(const char* arg);

		static void PlayerExitMap(const char* arg);

		static void HighwaymanMapFullyLoaded(const char* arg);
	}
}