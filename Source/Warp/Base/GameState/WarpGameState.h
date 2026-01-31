// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameState.h"
#include "WarpGameState.generated.h"

class ABaseUnitActor;
struct FUnitDefinition;
struct FUnitRecordDTO;
struct FUnitRecord;
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

	void SetupCombatUnitsArray(const int InNumberOfUnits);
	void AddCombatUnit(ABaseUnitActor* InCombatUnit);
	void SendCombatUnitsToClients();

	FOnWarpGameStateValid OnWarpGameStateValid;
	
protected:
	virtual void OnRep_MatchState() override;
	
	void HandleMatchLoading();
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;

	UFUNCTION()
	void OnRep_CombatUnits();

	void SetUnitsLoaded();

	bool bClientValidState_ = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_CombatUnits)
	TArray<ABaseUnitActor*> CombatUnits_;
	
};


