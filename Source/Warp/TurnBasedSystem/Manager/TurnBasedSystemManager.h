// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TurnBasedSystemManager.generated.h"

class AWarpGameState;
class UUnitBase;
/**
 * 
 */
UENUM()
enum class ETurnPhase : uint8
{
	None,
	InTurn
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTurnOrderUpdated, const TArray<uint32>& /*Order*/, uint32 /*CurrentId*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTurnPhaseChanged, ETurnPhase);


UCLASS()
class WARP_API UTurnBasedSystemManager : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void StartCombat();
	void AdvanceToNextUnit();
	void RebuildTurnOrder();

	UUnitBase* GetCurrentTurnUnit() const;
	ETurnPhase GetCurrentTurnPhase() const {return Phase;}
	const TArray<uint32>& GetTurnOrderUnitIds() const { return TurnOrderUnitIds; }
	
	int32 GetMinActionPointCost() const {return MinActionPointCost;}
	FOnTurnPhaseChanged OnTurnPhaseChanged;
	FOnTurnOrderUpdated OnTurnOrderUpdated;
	
protected:
	void BuildTurnOrder();
	void SortTurnOrder();
	void BeginTurnFor(uint32 UnitId);
	
	AWarpGameState* GetGameState() const;
	
	UFUNCTION()
	void OnRep_CurrentTurn();
	UFUNCTION()
	void OnRep_CurrentPhase();
	UFUNCTION()
	void OnRep_TurnOrder();

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTurn)
	uint32 CurrentTurnUnitId = 0;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentPhase)
	ETurnPhase Phase = ETurnPhase::None;
	UPROPERTY(ReplicatedUsing=OnRep_TurnOrder)
	TArray<uint32> TurnOrderUnitIds;
	

	uint32 MinActionPointCost = 1;
	int32 TurnIt = 0;
	bool bTurnsStarted = false;
};
