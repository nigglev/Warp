// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/Units/UnitBase.h"


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
	for (int i = 0; i < 4; i++)
		GetWarpGameState()->CreateUnitForNewPlayer(NewPlayer->PlayerState);
}

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}
