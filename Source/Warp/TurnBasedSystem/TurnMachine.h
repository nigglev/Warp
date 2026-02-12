// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Utils/RepAxialCoord.h"
#include "TurnMachine.generated.h"

struct FAxialAngle;
class ADefaultPlayerController;
class ABaseUnitActor;
class AWarpGameState;

UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
	WaitingForInput,
	WaitingForArrival
};

USTRUCT(BlueprintType)
struct FTurnState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	ETurnPhase Phase = ETurnPhase::WaitingForInput;
	
	UPROPERTY(BlueprintReadOnly)
	int32 ActiveUnitIndex = INDEX_NONE;
	
	FString ToString() const;
};

UCLASS()
class WARP_API UTurnMachine : public UObject
{
	GENERATED_BODY()

public:
	
	void CreateUnits();
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	const ABaseUnitActor* GetActiveUnit() const;
	ABaseUnitActor* GetActiveUnit();
	
	// Called on SERVER from PlayerController RPC
	bool RequestMove(const FRepAxialCoord& InTarget, const FAxialAngle& InAxialAngle);

	bool CanAcceptMove() const;

protected:
	
	AWarpGameState* GetOwner() const;
	
	UFUNCTION()
	void OnRep_CombatUnits();
	UFUNCTION()
	void OnRep_TurnState();
	
	void CheckLoaded() const;
	bool IsValidState() const;
	
	void SetNewActiveUnit(int32 InIndex);
	void SetWaitingForArrival();
	
	void OnUnitArrived(ABaseUnitActor* InUnit);
	
	UPROPERTY(ReplicatedUsing=OnRep_CombatUnits)
	TArray<ABaseUnitActor*> CombatUnits_;
	
	UPROPERTY(ReplicatedUsing=OnRep_TurnState)
	FTurnState TurnState_;
	
	UPROPERTY(Transient)
	ABaseUnitActor* PrevActiveUnit_;
};
