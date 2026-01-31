// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Warp/Actors/UnitActors/UnitActorFactory.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/ContentManagement/UnitStaticData/UnitDataTableRows.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"


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
	FName MS = GetMatchState();

	if (MS == MatchState::Loading)
	{
		if (CheckPlayersAndServerContentLoaded())
		{
			SetMatchState(MatchState::UnitCreation);
		}
	}
	if (MS == MatchState::UnitCreation)
	{
		if (CheckPlayersAndServerUnitCreation())
		{
			SetMatchState(MatchState::WaitingToStart);
		}
	}
	if (GetMatchState() == MatchState::WaitingToStart && GetWarpGameState()->GetMatchState() == MatchState::WaitingToStart)
	{
		StartBattle();
	}
}

void ADefaultGameMode::OnMatchStateSet()
{
	MG_LOG(ADefaultGameModeLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	Super::OnMatchStateSet();
	if (MatchState == MatchState::Loading)
	{
		HandleMatchLoading();
	}
	if (MatchState == MatchState::UnitCreation)
	{
		HandleUnitCreation();
	}
}

void ADefaultGameMode::HandleMatchLoading()
{
	CheckServerContentLoaded();
}

void ADefaultGameMode::HandleUnitCreation()
{
	RETURN_ON_FAIL(ADefaultGameModeLog, UnitsTable_);

	UUnitActorFactory* Factory = CreateUnitsFactory();
	RETURN_ON_FAIL(ADefaultGameModeLog, Factory);

	static const FString Context(TEXT("UnitsTable"));
	TArray<FUnitDataTableRows*> Rows;
	UnitsTable_->GetAllRows(Context, Rows);

	int N = 1;
	GetWarpGameState()->SetupCombatUnitsArray(N);

	for (const FUnitDataTableRows* Row : Rows)
	{
		if (!Row)
		{
			MG_ERROR(ADefaultGameModeLog, TEXT("Row %s not found"), *Row->UnitType.ToString());
			continue;
		}
		
		for(int i = 0; i < N; i++)
		{
			ABaseUnitActor* UnitActor = Factory->CreateByDTData(*Row, FTransform::Identity);
			if (!UnitActor)
			{
				MG_ERROR(ADefaultGameModeLog, TEXT("Failed to create actor %s"), *Row->UnitType.ToString());
			}

			GetWarpGameState()->AddCombatUnit(UnitActor);
		}
	}
	bUnitsCreated_ = true;
	GetWarpGameState()->SendCombatUnitsToClients();
}

void ADefaultGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ADefaultGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
}

bool ADefaultGameMode::StartBattle()
{
	RETURN_ON_FAIL_BOOL(LogGameMode, GetMatchState() == MatchState::WaitingToStart)
	StartMatch();
	return true;
}

#pragma region GameLoading

namespace ReadyToStartMatchErrors
{
	const FName NoServerContentReady = FName("NoServerContentReady");
	const FName PlayerArrayIsEmpty = FName("PlayerArrayIsEmpty");
	const FName PlayerIsNotReady = FName("PlayerIsNotReady");
}

void ADefaultGameMode::CheckServerContentLoaded()
{
	if (bServerContentReady_)
		return;

	auto PlayfabContentSubSystem =	UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultGameModeLog, PlayfabContentSubSystem);
	
	bServerContentReady_ = PlayfabContentSubSystem->IsContentLoaded();
	if (!bServerContentReady_)
	{
		PlayfabContentSubSystem->OnContentLoaded.AddWeakLambda(this, [this]()
		{
			this->CheckServerContentLoaded();
		});
	}
	MG_LOG(ADefaultGameModeLog, TEXT("bServerContentReady_: %s"), bServerContentReady_ ? TEXT("true") : TEXT("false"));
}

bool ADefaultGameMode::CheckPlayersAndServerContentLoaded()
{
	TValueOrError<void, FReadyToStartMatchError> ReadyValue = PlayersAndServerContentLoadValue();
	if (ReadyValue.HasValue())
		return true;

	if (LastPlayersAndServerLoadError_ != ReadyValue.GetError())
	{
		LastPlayersAndServerLoadError_ = ReadyValue.GetError();
		MG_LOG(ADefaultGameModeLog, TEXT("%s"), *LastPlayersAndServerLoadError_.ToString());
	}

	return false;
}

TValueOrError<void, ADefaultGameMode::FReadyToStartMatchError> ADefaultGameMode::PlayersAndServerContentLoadValue() const
{
	if (!bServerContentReady_)
		return MakeError(ReadyToStartMatchErrors::NoServerContentReady);
	if (GameState->PlayerArray.Num() == 0)
		return MakeError(ReadyToStartMatchErrors::PlayerArrayIsEmpty);
	
	for (APlayerState* PS : GameState->PlayerArray)
	{	
		if (AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS))
		{
			if (!WPS->IsClientContentLoaded())
			{
				return MakeError(ReadyToStartMatchErrors::PlayerIsNotReady, WPS->GetName());
			}
		}
	}
	return MakeValue();
}

bool ADefaultGameMode::CheckServerUnitCreation()
{
	return bUnitsCreated_;
}


bool ADefaultGameMode::CheckPlayersAndServerUnitCreation()
{
	TValueOrError<void, FReadyToStartMatchError> ReadyValue = PlayersAndServerUnitCreationValue();
	if (ReadyValue.HasValue())
		return true;

	if (LastPlayersAndServerLoadError_ != ReadyValue.GetError())
	{
		LastPlayersAndServerLoadError_ = ReadyValue.GetError();
		MG_LOG(ADefaultGameModeLog, TEXT("%s"), *LastPlayersAndServerLoadError_.ToString());
	}

	return false;
}

TValueOrError<void, ADefaultGameMode::FReadyToStartMatchError> ADefaultGameMode::
PlayersAndServerUnitCreationValue() const
{
	if (!bUnitsCreated_)
		return MakeError(ReadyToStartMatchErrors::NoServerContentReady);
	if (GameState->PlayerArray.Num() == 0)
		return MakeError(ReadyToStartMatchErrors::PlayerArrayIsEmpty);
	
	for (APlayerState* PS : GameState->PlayerArray)
	{	
		if (AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS))
		{
			if (!WPS->IsClientUnitsLoaded())
			{
				return MakeError(ReadyToStartMatchErrors::PlayerIsNotReady, WPS->GetName());
			}
		}
	}
	return MakeValue();
}

#pragma endregion

UUnitActorFactory* ADefaultGameMode::CreateUnitsFactory()
{	
	UUnitActorFactory* Factory = NewObject<UUnitActorFactory>(this);
	return Factory;
}

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}



