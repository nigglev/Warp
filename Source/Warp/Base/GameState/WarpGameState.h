// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameStateBase.h"
#include "Warp/CombatMap/CombatMap.h"
#include "WarpGameState.generated.h"

class UUnitDataSubsystem;
class UTurnBasedSystemManager;
class UUnitBase;
class UCombatMap;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitsReplicatedSignature, const TArray<UUnitBase*>&);
/**
 * 
 */

UCLASS()
class WARP_API AWarpGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AWarpGameState();
	void PreLoginInit();

	void CreateUnitAtRandomPosition(uint8 InUnitTypeID);
	bool CreateUnitAt(const uint8 InUnitTypeID, const FIntPoint& InGridPosition,
	const FUnitRotation& Rotation, UUnitBase*& OutUnit);
	
	uint32 GetMapGridSize() const;
	uint32 GetMapTileSize() const;
	TArray<UUnitBase*> GetActiveUnits() {return ActiveUnits;}
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const {return TurnManager;}
	
	UUnitBase* FindUnitByCombatID(const uint32 InCombatID) const;
	bool CheckPositionForUnitWithCombatMap(const FIntPoint& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const;

	FOnUnitsReplicatedSignature OnUnitsReplicated;
	
protected:
	void AddNewUnit(UUnitBase* InNewUnit);
	UUnitBase* CreateUnit(const uint8 InUnitTypeID);
	UUnitDataSubsystem* GetUnitDataSubsystem(const UObject* WorldContext);
	
	UFUNCTION()
	void OnRep_SpaceCombatGrid();
	UFUNCTION()
	void OnRep_TurnBasedSystemManager();
	UFUNCTION()
	void OnRep_ActiveUnits();
	

	UPROPERTY(ReplicatedUsing=OnRep_SpaceCombatGrid)
	UCombatMap* StaticCombatMap;
	UPROPERTY(ReplicatedUsing=OnRep_TurnBasedSystemManager)
	UTurnBasedSystemManager* TurnManager = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_ActiveUnits)
	TArray<UUnitBase*> ActiveUnits;

	uint32 NextUnitCombatID;
};

