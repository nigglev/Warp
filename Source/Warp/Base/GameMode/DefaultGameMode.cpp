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
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/UEBase/PlayerController/BP_DefaultPlayerController"));
	if (PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void ADefaultGameMode::StartPlay()
{
	Super::StartPlay();
	MG_FUNC_LABEL(ADefaultGameModeLog);
	GetWarpGameState()->PreLoginInit();
}

void ADefaultGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!HasAuthority()) return;
	if (bInitialUnitsSpawned) return;

	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);

	if (Sys->IsUnitCatalogReady())
		SpawnInitialUnitsOnce();
	else
		Sys->OnUnitCatalogReady.AddUniqueDynamic(this, &ADefaultGameMode::HandleUnitCatalogReady);

}

void ADefaultGameMode::HandleUnitCatalogReady(bool bSuccess)
{
	if (!HasAuthority() || bInitialUnitsSpawned) return;
	if (!bSuccess) return;

	SpawnInitialUnitsOnce();
}

void ADefaultGameMode::SpawnInitialUnitsOnce()
{
	if (bInitialUnitsSpawned) return;
	bInitialUnitsSpawned = true;

	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	FUnitRecord Record = Sys->GetBasicUnitRecord();
	
	for (int i = 0; i < 4; i++)
	{
		GetWarpGameState()->CreateUnitAtRandomPosition(Record);
	}
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
