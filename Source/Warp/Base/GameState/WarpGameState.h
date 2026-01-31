// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameState.h"
#include "WarpGameState.generated.h"

class UTurnMachine;
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
UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
	WaitingForInput,
	WaitingForArrival
};

UCLASS()
class WARP_API AWarpGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AWarpGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	void SetupCombatUnitsArray(const int InNumberOfUnits);
	void AddCombatUnit(ABaseUnitActor* InCombatUnit);
	void SendCombatUnitsToClients();
	void SetUnitsLoaded();

	UTurnMachine* GetTurnMachine() const { return TurnMachine_; }
	TArray<ABaseUnitActor*> GetCombatUnits() const { return CombatUnits_; }
	
	int32 GetActiveUnitIndex() const { return ActiveUnitIndex_; }
	void SetActiveUnitIndex(int32 InActiveUnitIndex) {ActiveUnitIndex_ = InActiveUnitIndex;}
	
	ETurnPhase GetTurnPhase() const { return TurnPhase_; }
	void SetTurnPhase(ETurnPhase InTurnPhase) {TurnPhase_ = InTurnPhase;}
	
	ABaseUnitActor* GetActiveUnit() const
	{
		return CombatUnits_.IsValidIndex(ActiveUnitIndex_) ? CombatUnits_[ActiveUnitIndex_] : nullptr;
	}

	FOnWarpGameStateValid OnWarpGameStateValid;
	
protected:
	virtual void OnRep_MatchState() override;
	virtual void HandleMatchIsWaitingToStart() override;


	UFUNCTION()
	void OnRep_CombatUnits();
	UFUNCTION()
	void OnRep_TurnState();

	bool bClientValidState_ = false;

	UPROPERTY(Transient)
	TObjectPtr<UTurnMachine> TurnMachine_;
	UPROPERTY(ReplicatedUsing=OnRep_CombatUnits)
	TArray<ABaseUnitActor*> CombatUnits_;
	UPROPERTY(ReplicatedUsing=OnRep_TurnState)
	int32 ActiveUnitIndex_ = INDEX_NONE;
	UPROPERTY(ReplicatedUsing=OnRep_TurnState)
	ETurnPhase TurnPhase_ = ETurnPhase::WaitingForInput;
	
};


