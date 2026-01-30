// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
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
		SetMatchState(MatchState::WaitingToStart);
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

	if (MS == MatchState::WaitingToStart)
	{
		// Check to see if we should start the match
		if (CheckLoading())
		{
			UE_LOG(LogGameMode, Log, TEXT("GameMode returned Loaded"));
			StartBattle();
		}
	}
}

void ADefaultGameMode::OnMatchStateSet()
{
	MG_LOG(ADefaultGameModeLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	Super::OnMatchStateSet();
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchHasLoading();
	}
}

void ADefaultGameMode::HandleMatchHasLoading()
{
	CheckServerContentLoading();
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

void ADefaultGameMode::ReturnToCampaignMap()
{
	RETURN_ON_FAIL(ADefaultGameModeLog, MapNodeType_ != EMapNodeType::Undefined);
	RETURN_ON_FAIL(ADefaultGameModeLog, !CampaignMap.IsNone());
	
	UGameplayStatics::OpenLevel(this, CampaignMap, true, 
		 FString::Printf(TEXT("NodePosition=%s"), *NodePosition_.ToString()));
}