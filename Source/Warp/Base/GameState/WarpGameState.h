// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/CombatMap(Deprecated)/CombatMap.h"
#include "WarpGameState.generated.h"

struct FUnitDefinition;
struct FUnitRecordDTO;
struct FUnitRecord;
class UTurnBasedSystemManager;
class UUnitBase;
class UCombatMap;
class AWarpGameState;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWarpGameStateValid, AWarpGameState*);
DECLARE_MULTICAST_DELEGATE(FOnCombatStarted);
/**
 * 
 */

UCLASS()
class WARP_API AWarpGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AWarpGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	FOnWarpGameStateValid OnWarpGameStateValid;
	
protected:
	virtual void OnRep_MatchState() override;
	
	void HandleMatchHasLoading();
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;

	void CheckValidState();

	bool bClientValidState_ = false;
};

