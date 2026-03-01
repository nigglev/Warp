// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "Kismet/GameplayStatics.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
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

void ADefaultGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	FString MapType = UGameplayStatics::ParseOption(Options, TEXT("MapType"));
	MG_LOG(ADefaultGameModeLog, TEXT("Options: %s"), *Options);
	
	if (!MapType.IsEmpty())
	{
		int64 iMapNodeType = StaticEnum<EMapNodeType>()->GetValueByNameString(MapType);
		MG_COND_ERROR_SHORT(ADefaultGameModeLog, iMapNodeType == INDEX_NONE);
		if (iMapNodeType != INDEX_NONE)
		{
			MapNodeType_ = static_cast<EMapNodeType>(iMapNodeType);
		}
	}
	
	FString NodePositionStr = UGameplayStatics::ParseOption(Options, TEXT("NodePosition"));
	if (!NodePositionStr.IsEmpty())
	{
		bool bNodePositionParsed = NodePosition_.InitFromString(NodePositionStr);
		MG_COND_ERROR_SHORT(ADefaultGameModeLog, !bNodePositionParsed);
		MG_LOG(ADefaultGameModeLog, TEXT("NodePosition: %s"), *NodePosition_.ToString());
	}
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
	MG_FUNC_LABEL(ADefaultGameModeLog);

	if (bServerContentReady_)
		return;

	auto PlayfabContentSubSystem =	UWarpContentSubSystem::Get(this);
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

bool ADefaultGameMode::CheckServerUnitCreation() const
{
	AWarpGameState* GS = GetWarpGameState();
	return GS == nullptr ? false : GS->IsUnitsCreated();
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

TValueOrError<void, ADefaultGameMode::FReadyToStartMatchError> 
ADefaultGameMode::PlayersAndServerUnitCreationValue() const
{
	if (!CheckServerUnitCreation())
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

AWarpGameState* ADefaultGameMode::GetWarpGameState() const
{
	AWarpGameState* GS = GetGameState<AWarpGameState>();
	MG_COND_ERROR_SHORT(ADefaultGameModeLog, GS == nullptr);
	return GS;
}

void ADefaultGameMode::ReturnToCampaignMap()
{
	RETURN_ON_FAIL(ADefaultGameModeLog, MapNodeType_ != EMapNodeType::Undefined);
	RETURN_ON_FAIL(ADefaultGameModeLog, !CampaignMap.IsNone());
	
	UGameplayStatics::OpenLevel(this, CampaignMap, true, 
		 FString::Printf(TEXT("NodePosition=%s"), *NodePosition_.ToString()));
}


