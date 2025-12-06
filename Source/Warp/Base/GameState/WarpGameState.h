// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "Warp/CombatMap/CombatMap.h"
#include "WarpGameState.generated.h"

struct FUnitDefinition;
struct FUnitRecordDTO;
struct FUnitRecord;
class UTurnBasedSystemManager;
class UUnitBase;
class UCombatMap;
class AWarpGameState;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWarpGameStateValid, AWarpGameState*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitsReplicatedSignature, const TArray<UUnitBase*>&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatStarted);
/**
 * 
 */

UCLASS()
class WARP_API AWarpGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AWarpGameState();
	void PreLoginInit();

	virtual void PostInitializeComponents() override;

	void CreateUnitAtRandomPosition(const FUnitDefinition* InUnitDefinition, const EUnitAffiliation InAffiliation);
	bool CreateUnitAt(const FUnitRecord& InUnitRecord, const EUnitAffiliation InAffiliation, const FIntVector2& InGridPosition,
	const FUnitRotation& Rotation, UUnitBase*& OutUnit);
	bool MoveUnitTo(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition);
	
	uint32 GetMapGridSize() const;
	uint32 GetMapTileSize() const;
	UUnitBase* GetUnitByID(uint32 InUnitID);
	TArray<UUnitBase*> GetActiveUnits() {return ActiveUnits;}
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const {return TurnManager;}
	
	UUnitBase* FindUnitByCombatID(const uint32 InCombatID) const;
	bool CheckPositionForUnitWithCombatMap(const FIntVector2& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const;
	
	bool IsCombatStarted() const { return bCombatStarted; }
	void SetCombatStarted(bool bStarted);

	bool IsClientValidState() const { return StaticCombatMap != nullptr && TurnManager != nullptr;}

	FOnWarpGameStateValid OnWarpGameStateValid;
	FOnUnitsReplicatedSignature OnUnitsReplicated;
	FOnCombatStarted OnCombatStarted;
	
protected:
	void ProcessNewUnit(UUnitBase* InNewUnit);
	UUnitBase* CreateUnit(const FUnitDefinition* InUnitDefinition, const EUnitAffiliation InAffiliation);

	void CheckValidState();

	UFUNCTION()
	void OnRep_SpaceCombatGrid();
	UFUNCTION()
	void OnRep_TurnBasedSystemManager();
	UFUNCTION()
	void OnRep_ActiveUnits();
	UFUNCTION()
	void OnRep_CombatStarted();
	

	UPROPERTY(ReplicatedUsing=OnRep_SpaceCombatGrid)
	UCombatMap* StaticCombatMap = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_TurnBasedSystemManager)
	UTurnBasedSystemManager* TurnManager = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_ActiveUnits)
	TArray<UUnitBase*> ActiveUnits;
	UPROPERTY(ReplicatedUsing=OnRep_CombatStarted)
	bool bCombatStarted = false;

	uint32 NextUnitCombatID;

	bool bClientValidState_ = false;
};

