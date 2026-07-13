#pragma once

namespace PArroyo {
	void PouyaFunction(std::monostate);
}

bool InitializeSharedGameVariables();

void F4SEMessageCallback(F4SE::MessagingInterface::Message* myMessage);

void GameDataLoaded();
void GameDataReady();

void InitializeCustomConsoleCommands();