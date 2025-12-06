// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
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


void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetupServerContent();
}

void ADefaultGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	// if (bMainPlayerSpawned && bAISpawned) return;
	//
	// UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	//
	// if (Sys->IsUnitCatalogReady())
	// {
	// 	SpawnPlayerMainShip();
	// 	SpawnAIShips(AINumber);
	// }
	// else
	// 	Sys->OnUnitCatalogReady.AddUniqueDynamic(this, &ADefaultGameMode::HandleUnitCatalogReady);
}


void ADefaultGameMode::SetupServerContent()
{
	UGameInstance* GI = GetGameInstance();
	RETURN_ON_FAIL(ADefaultGameModeLog, GI);

	Content_ =	GI->GetSubsystem<UWarpPlayfabContentSubSystem>();

	RETURN_ON_FAIL(ADefaultGameModeLog, Content_);

	if (Content_->AreUnitsLoaded())
	{
		HandleUnitsReadyServer();
	}
	else
	{
		Content_->OnUnitsLoaded.AddDynamic(this, &ADefaultGameMode::HandleUnitsReadyServer);
	}
}


void ADefaultGameMode::HandleUnitsReadyServer()
{
	bServerContentReady_ = true;
	CheckStartConditions();
}

void ADefaultGameMode::CheckStartConditions()
{
	if (HasMatchStarted())
	{
		return;
	}

	//StartMatch();
}

namespace ReadyToStartMatchErrors
{
	const FName NoServerContentReady = FName("NoServerContentReady");
	const FName PlayerArrayIsEmpty = FName("PlayerArrayIsEmpty");
	const FName PlayerIsNotReady = FName("PlayerIsNotReady");
}

TValueOrError<void, ADefaultGameMode::FReadyToStartMatchError> ADefaultGameMode::ReadyToStartMatchValue() const
{
	if (!bServerContentReady_)
		return MakeError(ReadyToStartMatchErrors::NoServerContentReady);
	if (GameState->PlayerArray.Num() == 0)
		return MakeError(ReadyToStartMatchErrors::PlayerArrayIsEmpty);
	
	for (APlayerState* PS : GameState->PlayerArray)
	{	
		if (AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS))
		{
			if (!WPS->bClientContentReady)
			{
				return MakeError(ReadyToStartMatchErrors::PlayerIsNotReady, WPS->GetName());
			}
		}
	}
	return MakeValue();
}

bool ADefaultGameMode::ReadyToStartMatch_Implementation()
{
	TValueOrError<void, FReadyToStartMatchError> ReadyValue = ADefaultGameMode::ReadyToStartMatchValue();
	if (ReadyValue.HasValue())
		return true;

	if (LastReadyToStartMatchError_ != ReadyValue.GetError())
	{
		LastReadyToStartMatchError_ = ReadyValue.GetError();
		MG_LOG(ADefaultGameModeLog, TEXT("%s"), *LastReadyToStartMatchError_.ToString());
	}

	return false;	
}

void ADefaultGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	MG_COND_LOG(ADefaultGameModeLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("Match has started: building map and spawning starting units."));
	RETURN_ON_FAIL(ADefaultGameModeLog, Content_);
	SpawnPlayerMainShip();
	
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
	RETURN_ON_FAIL(ADefaultGameModeLog, Content_);
	const FUnitDefinition* CorvetteUnitDef = Content_->GetUnitDefinition(FName(TEXT("Corvette")));
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

