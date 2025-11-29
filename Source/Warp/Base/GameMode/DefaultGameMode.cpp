// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Base/GameInstanceSubsystem/UnitDataSubsystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/Units/UnitBase.h"
#include "Warp/UnitStaticData/PlayFabUnitTypes.h"


DEFINE_LOG_CATEGORY_STATIC(ADefaultGameModeLog, Log, All);

ADefaultGameMode::ADefaultGameMode()
{
	GameStateClass = AWarpGameState::StaticClass();
	PlayerControllerClass = ADefaultPlayerController::StaticClass();
	HUDClass = ADefaultWarpHUD::StaticClass();
	DefaultPawnClass = ATacticalCameraPawn::StaticClass();
	PlayerControllerClass = ADefaultPlayerController::StaticClass();
}

void ADefaultGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (bMainPlayerSpawned && bAISpawned) return;

	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);

	if (Sys->IsUnitCatalogReady())
	{
		SpawnPlayerMainShip();
		SpawnAIShips(AINumber);
	}
	else
		Sys->OnUnitCatalogReady.AddUniqueDynamic(this, &ADefaultGameMode::HandleUnitCatalogReady);

}

void ADefaultGameMode::HandleUnitCatalogReady(bool bSuccess)
{
	if (!HasAuthority() || (bMainPlayerSpawned && bAISpawned)) return;
	if (!bSuccess) return;

	SpawnPlayerMainShip();
	SpawnAIShips(AINumber);
}

void ADefaultGameMode::SpawnPlayerMainShip()
{
	if (bMainPlayerSpawned) return;
	
	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	FUnitRecord Record = Sys->GetPlayerMainShipRecord();
	GetWarpGameState()->CreateUnitAtRandomPosition(Record, EUnitAffiliation::Player);
	
	bMainPlayerSpawned = true;
}

void ADefaultGameMode::SpawnAIShips(int InAINumber)
{
	if (bAISpawned) return;
	
	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	FUnitRecord Record = Sys->GetCorvetteRecord();

	for (int i = 0; i < InAINumber; i++)
		GetWarpGameState()->CreateUnitAtRandomPosition(Record, EUnitAffiliation::Enemy);
	
	bAISpawned = true;
}

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}

UUnitDataSubsystem* ADefaultGameMode::GetUnitDataSubsystem(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UUnitDataSubsystem>();
		}
	}
	return nullptr;
}
