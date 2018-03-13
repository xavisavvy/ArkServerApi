#pragma once
#include "DeathmatchEvent.h"
#include "../../EventManager/EventManager/Public/EventManager.h"
#pragma comment(lib, "AAEventManager.lib")
#pragma comment(lib, "ArkApi.lib")
#include <fstream>
#include "json.hpp"

class DeathMatch : Event
{
public:
	virtual void InitConfig(const FString& Map)
	{
		if (!GetSpawnsSet())
		{
			Log::GetLog()->warn("Spawns Set");
			InitDefaults(L"DeathMatch", L"Epidemic", 2, true, FVector(0, 0, 0), 60000);
			/*const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/DeathMatchEvent/" + Map.ToString() + ".json";
			std::ifstream file { config_path };
			if (!file.is_open()) throw std::runtime_error(fmt::format("Can't open {}.json", Map.ToString().c_str()).c_str());
			nlohmann::json config;
			file >> config;
			auto DMSpawns = config["Deathmatch"]["Spawns"];
			for (const auto& PermColours : DMSpawns)
			{
			config = PermColours["Position"];
			AddSpawn(FVector(config[0], config[1], config[2]));
			}
			file.close();*/
		}
		Reset();
		AddTime(60);

	}

	virtual void Update()
	{
		Log::GetLog()->warn("Update() State({})", (int)GetState());
		switch (GetState())
		{
		case EventState::WaitingForPlayers:
			if (TimePassed())
			{
				if (UpCount() == 4)
				{
					ResetCount();
					if (EventManager::GetEventManager()->GetEventPlayerCount() < 2)
					{
						ArkApi::GetApiUtils().SendChatMessageToAll(GetServerName(), L"[Event] {} Failed to start needed 2 Players", *GetName());
						SetState(EventState::Finnished);
					}
					else SetState(EventState::TeleportingPlayers);
				}
				AddTime(60);
			}
			break;
		case EventState::TeleportingPlayers:
			EventManager::TeleportEventPlayers(false, true, true, GetSpawns(), GetSpawns(EventTeam::Blue));
			EventManager::GetEventManager()->SendChatMessageToAllEventPlayers(GetServerName(), L"[Event] {} Starting in 30 Seconds", *GetName());
			AddTime(30);
			SetState(EventState::WaitForFight);
			break;
		case EventState::WaitForFight:
			if (TimePassed())
			{
				EventManager::GetEventManager()->SendChatMessageToAllEventPlayers(GetServerName(), L"[Event] {} Started Kill or Be Killed!", *GetName());
				SetState(EventState::Fighting);
			}
			break;
		case EventState::Fighting:
			break;
		case EventState::Rewarding:
			break;
		}
	}
};

DeathMatch* DmEvent;

DECLARE_HOOK(UWorld_InitWorld, void, UWorld*, DWORD64);

void InitEvent()
{
	Log::Get().Init("Deathmatch Event");
	DmEvent = new DeathMatch();
	ArkApi::GetHooks().SetHook("UWorld.InitWorld", &Hook_UWorld_InitWorld, &UWorld_InitWorld_original);
}

void RemoveEvent()
{
	ArkApi::GetHooks().DisableHook("UWorld.InitWorld", &Hook_UWorld_InitWorld);
	EventManager::RemoveEvent((Event*)DmEvent);
}

void Hook_UWorld_InitWorld(UWorld* world, DWORD64 ivs)
{
	EventManager::AddEvent((Event*)DmEvent);
	Log::GetLog()->warn("Event Added");

	UWorld_InitWorld_original(world, ivs);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitEvent();
		break;
	case DLL_PROCESS_DETACH:
		RemoveEvent();
		break;
	}
	return TRUE;
}