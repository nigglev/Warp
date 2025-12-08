// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/Base/GameInstanceSubsystem/WarpPlayfabContentSubSystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/Units/UnitBase.h"


DEFINE_LOG_CATEGORY_STATIC(ADefaultGameModeLog, Log, All);

ADefaultGameMode::ADefaultGameMode()
{
	GameStateClass = AWarpGameState::StaticClass();
	PlayerControllerClass = ADefaultPlayerController::StaticClass();
	HUDClass = ADefaultWarpHUD::StaticClass();
	DefaultPawnClass = ATacticalCameraPawn::StaticClass();
	PlayerStateClass = AWarpPlayerState::StaticClass();

	bDelayedStart = true;
}

void ADefaultGameMode::StartPlay()
{
	if (MatchState == MatchState::EnteringMap)
	{
		SetMatchState(MatchState::Loading);
	}
}

void ADefaultGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ADefaultGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::Loading)
	{
		// Check to see if we should start the match
		if (CheckLoading())
		{
			UE_LOG(LogGameMode, Log, TEXT("GameMode returned Loaded"));
			SetMatchState(MatchState::UnitCreating);
		}
	}
	else if (GetMatchState() == MatchState::UnitCreating)
	{
		// Check to see if we should start the match
		if (CheckUnitCreating())
		{
			UE_LOG(LogGameMode, Log, TEXT("GameMode returned Loaded"));
			SetMatchState(MatchState::WaitingToStart);
		}
	}
}

void ADefaultGameMode::OnMatchStateSet()
{
	MG_LOG(ADefaultGameModeLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	Super::OnMatchStateSet();
	if (MatchState == MatchState::Loading)
	{
		HandleMatchHasLoading();
	}
}

void ADefaultGameMode::HandleMatchHasLoading()
{
	CheckServerContentLoading();
}

#pragma region GameLoading

void ADefaultGameMode::CheckServerContentLoading()
{
	if (bServerContentReady_)
		return;

	auto Content =	UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultGameModeLog, Content);
	
	bServerContentReady_ = Content->AreUnitsLoaded();
	if (!bServerContentReady_)
	{
		Content->OnUnitsLoaded.AddWeakLambda(this, [this]()
		{
			this->CheckServerContentLoading();
		});
	}
	MG_LOG(ADefaultGameModeLog, TEXT("bServerContentReady_: %s"), bServerContentReady_ ? TEXT("true") : TEXT("false"));
}

bool ADefaultGameMode::CheckLoading()
{
	TValueOrError<void, FReadyToStartMatchError> ReadyValue = PlayersAndServerLoadValue();
	if (ReadyValue.HasValue())
		return true;

	if (LastPlayersAndServerLoadError_ != ReadyValue.GetError())
	{
		LastPlayersAndServerLoadError_ = ReadyValue.GetError();
		MG_LOG(ADefaultGameModeLog, TEXT("%s"), *LastPlayersAndServerLoadError_.ToString());
	}

	return false;
}

namespace ReadyToStartMatchErrors
{
	const FName NoServerContentReady = FName("NoServerContentReady");
	const FName PlayerArrayIsEmpty = FName("PlayerArrayIsEmpty");
	const FName PlayerIsNotReady = FName("PlayerIsNotReady");
}

TValueOrError<void, ADefaultGameMode::FReadyToStartMatchError> ADefaultGameMode::PlayersAndServerLoadValue() const
{
	if (!bServerContentReady_)
		return MakeError(ReadyToStartMatchErrors::NoServerContentReady);
	if (GameState->PlayerArray.Num() == 0)
		return MakeError(ReadyToStartMatchErrors::PlayerArrayIsEmpty);
	
	for (APlayerState* PS : GameState->PlayerArray)
	{	
		if (AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS))
		{
			if (!WPS->IsClientLoaded())
			{
				return MakeError(ReadyToStartMatchErrors::PlayerIsNotReady, WPS->GetName());
			}
		}
	}
	return MakeValue();
}
#pragma endregion

#pragma region UnitCreating
void ADefaultGameMode::HandleMatchHasUnitCreating()
{
}

bool ADefaultGameMode::CheckUnitCreating()
{
	return false;
}
#pragma endregion

bool ADefaultGameMode::StartBattle()
{
	RETURN_ON_FAIL_BOOL(LogGameMode, GetMatchState() == MatchState::WaitingToStart)
	StartMatch();
	return true;
}

void ADefaultGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
	SpawnPlayerMainShip();	
}

void ADefaultGameMode::SpawnPlayerMainShip()
{
	auto Content =	UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultGameModeLog, Content);

	const FUnitDefinition* CorvetteUnitDef = Content->GetUnitDefinition(FName(TEXT("Corvette")));
	RETURN_ON_FAIL_T(ADefaultGameModeLog, CorvetteUnitDef, TEXT("No 'corvette' unit definition in content"));

	GetWarpGameState()->CreateUnitAtRandomPosition(CorvetteUnitDef, EUnitAffiliation::Player);
	// if (bMainPlayerSpawned) return;
	//
	// UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	// FUnitRecord Record = Sys->GetPlayerMainShipRecord();
	// GetWarpGameState()->CreateUnitAtRandomPosition(Record, EUnitAffiliation::Player);
	//
	// bMainPlayerSpawned = true;
}

void ADefaultGameMode::SpawnAIShips(int InAINumber)
{
// 	if (bAISpawned) return;
// 	
// 	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
// 	FUnitRecord Record = Sys->GetCorvetteRecord();
//
// 	for (int i = 0; i < InAINumber; i++)
// 		GetWarpGameState()->CreateUnitAtRandomPosition(Record, EUnitAffiliation::Enemy);
// 	
// 	bAISpawned = true;
}

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}

