// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameMode.generated.h"

class UWarpPlayfabContentSubSystem;
class ADefaultPlayerController;
class ACombatMapManager;
class AWarpGameState;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	ADefaultGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;
	
	void CheckStartConditions();
	
protected:
	void SetupServerContent();

	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleMatchHasStarted() override;
	
	UFUNCTION()
	void HandleUnitCatalogReady(bool bSuccess);
	UFUNCTION()
	void HandleUnitsReadyServer();
	
	void SpawnPlayerMainShip();
	void SpawnAIShips(int InAINumber);

	AWarpGameState* GetWarpGameState() const;

	UWarpPlayfabContentSubSystem* Content_;
	
	bool bServerContentReady_ = false;
	
	bool bInitialUnitsSpawned = false;
	bool bMainPlayerSpawned = false;
	bool bAISpawned = false;
	int AINumber = 5;
};





