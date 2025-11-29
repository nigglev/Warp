// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameMode.generated.h"

class UUnitDataSubsystem;
class ADefaultPlayerController;
class ACombatMapManager;
class AWarpGameState;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADefaultGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
protected:
	UFUNCTION()
	void HandleUnitCatalogReady(bool bSuccess);
	void SpawnPlayerMainShip();
	void SpawnAIShips(int InAINumber);

	AWarpGameState* GetWarpGameState() const;
	UUnitDataSubsystem* GetUnitDataSubsystem(const UObject* WorldContext);

	bool bInitialUnitsSpawned = false;
	bool bMainPlayerSpawned = false;
	bool bAISpawned = false;
	int AINumber = 5;
};



