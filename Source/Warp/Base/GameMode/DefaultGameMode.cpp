// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/ContentManagement/UnitStaticData/UnitDataTableRows.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/Units(Deprecated)/UnitBase.h"


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
		// Check to see if we should start the match
		if (CheckLoading())
		{
			UE_LOG(LogGameMode, Log, TEXT("GameMode returned Loaded"));
			SetMatchState(MatchState::UnitCreation);
		}
		if (CheckUnitCreation())
		{
			UE_LOG(LogGameMode, Log, TEXT("Units Created"));
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
	if (MatchState == MatchState::UnitCreation)
	{
		HandleUnitCreation();
	}
}

void ADefaultGameMode::HandleMatchHasLoading()
{
	CheckServerContentLoading();
}

void ADefaultGameMode::HandleUnitCreation()
{
	auto Content =	UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultGameModeLog, Content);
	RETURN_ON_FAIL(ADefaultGameModeLog, UnitsTable_);

	static const FString Context(TEXT("UnitsTable"));
	TArray<FUnitDataTableRows*> Rows;
	UnitsTable_->GetAllRows(Context, Rows);

	for (const FUnitDataTableRows* Row : Rows)
	{
		if (!Row || Row->UnitType.IsNone())
		{
			continue;
		}
		
		const FUnitDescription& UnitDesc = Content->GetDescription<FUnitDescription>(Row->UnitType);
		UClass* UnitClass = Row->UnitActor.LoadSynchronous();
		MG_LOG(ADefaultGameModeLog, TEXT("Row UnitType=%s -> got description."), *Row->UnitType.ToString());

		if (!UnitClass || !UnitClass->IsChildOf(ABaseUnitActor::StaticClass()))
		{
			return;
		}

		UWorld* World = GetWorld();
		RETURN_ON_FAIL(ADefaultGameModeLog, World);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABaseUnitActor* UnitActor = World->SpawnActor<ABaseUnitActor>(UnitClass, FTransform::Identity, Params);
		RETURN_ON_FAIL(ADefaultGameModeLog, UnitActor);
		FUnitSize Size(UnitDesc.UnitSize);
		UnitActor->SetUnitActorSize(Size);
	}

	bUnitsCreated_ = true;
}

void ADefaultGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ADefaultGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	StartBattle();
}

bool ADefaultGameMode::StartBattle()
{
	RETURN_ON_FAIL_BOOL(LogGameMode, GetMatchState() == MatchState::WaitingToStart)
	StartMatch();
	return true;
}

#pragma region GameLoading

void ADefaultGameMode::CheckServerContentLoading()
{
	if (bServerContentReady_)
		return;

	auto Content =	UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultGameModeLog, Content);
	
	bServerContentReady_ = Content->IsClientDataLoaded();
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


bool ADefaultGameMode::CheckUnitCreation()
{
	return bUnitsCreated_;
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

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}

