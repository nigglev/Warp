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

DECLARE_MULTICAST_DELEGATE(FOnCombatStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnitSelected, ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitStartMoving, ABaseUnitActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitArrived, ABaseUnitActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewRound, uint32);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCombatUnitsChanged, const TArray<ABaseUnitActor*>&, int32, int32);

UCLASS()
class WARP_API AWarpGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AWarpGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;

	void SetUnitsLoaded();
	
	bool IsUnitsCreated() const { return bUnitsCreated_; }

	const UTurnMachine* GetTurnMachine() const { return TurnMachine_; }
	UTurnMachine* GetTurnMachine() { return TurnMachine_; }

	void BroadcastUIInfo() const;

	FOnMatchStateChanged OnMatchStateChanged;
	FOnUnitSelected OnUnitSelected;
	FOnUnitStartMoving OnUnitStartMoving;
	FOnUnitArrived OnUnitArrived;
	FOnNewRound OnNewRound;

	FOnCombatUnitsChanged OnTurnOrderChanged;
	
protected:
	virtual void OnRep_MatchState() override;
	void HandleUnitCreation();
	virtual void HandleMatchIsWaitingToStart() override;


	bool bClientValidState_ = false;
	bool bUnitsCreated_ = false;

	UPROPERTY(Replicated)
	TObjectPtr<UTurnMachine> TurnMachine_;	
};


