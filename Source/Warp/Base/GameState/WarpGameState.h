// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGLogTypes.h"
#include "GameFramework/GameStateBase.h"
#include "Warp/CombatMap/CombatMap.h"
#include "WarpGameState.generated.h"

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
	
	uint32 GetMapGridSize() const;
	uint32 GetMapTileSize() const;

	void CreateUnitForNewPlayer(APlayerState* OwnerPS);
	bool TryCreateUnitAtForOwner(APlayerState* OwnerPS, const FIntPoint& CenterGrid, const FUnitRotation& Rotation,
		const FUnitSize& Size, UUnitBase*& OutUnit);
	
	TArray<UUnitBase*> GetActiveUnits() {return ActiveUnits;}
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const {return TurnManager;}
	UUnitBase* FindUnitByID(const uint32 InID) const;
	bool CheckPositionForUnitWithCombatMap(const FIntPoint& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const;

	FOnUnitsReplicatedSignature OnUnitsReplicated;
protected:
	void AddNewUnit(UUnitBase* InNewUnit);
	
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

	uint32 NextUnitID;
};
