#include "API/ARK/Ark.h"

#include "PointRewards.h"

#include "Utils.h"

#include "Stats.h"

#include "Hooks.h"

#include "Commands.h"

#include "Reload.h"


#pragma comment(lib, "AsaApi.lib")



void OnServerReady()
{
	Log::GetLog()->info("PointRewards Initialized");

	ReadConfig();
	LoadDatabase();
	AddOrRemoveCommands();
	AddReloadCommands();
	SetHooks();
}

DECLARE_HOOK(AShooterGameMode_BeginPlay, void, AShooterGameMode*);
void Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this)
{
	AShooterGameMode_BeginPlay_original(_this);

	OnServerReady();
}

extern "C" __declspec(dllexport) void Plugin_Init()
{
	Log::Get().Init(PROJECT_NAME);

	AsaApi::GetHooks().SetHook("AShooterGameMode.BeginPlay()", Hook_AShooterGameMode_BeginPlay, &AShooterGameMode_BeginPlay_original);

	if (AsaApi::GetApiUtils().GetStatus() == AsaApi::ServerStatus::Ready)
		OnServerReady();
}

extern "C" __declspec(dllexport) void Plugin_Unload()
{
	AsaApi::GetHooks().DisableHook("AShooterGameMode.BeginPlay()", Hook_AShooterGameMode_BeginPlay);

	AddOrRemoveCommands(false);
	AddReloadCommands(false);
	SetHooks(false);
}