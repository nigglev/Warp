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

	struct FReadyToStartMatchError
	{
		FName ErrorName;
		FString ErrorDescription;
		
		FReadyToStartMatchError() = default;
		FReadyToStartMatchError(FName InErrorName) : ErrorName(InErrorName) {}
		FReadyToStartMatchError(FName InErrorName, const FString& InErrorDescription) : ErrorName(InErrorName), ErrorDescription(InErrorDescription) {}
		
		bool operator==(const FReadyToStartMatchError& InOther) const
		{
			return InOther.ErrorName == ErrorName && InOther.ErrorDescription == ErrorDescription;
		}
		bool operator!=(const FReadyToStartMatchError& InOther) const
		{
			return !(*this == InOther);
		}

		FString ToString() const
		{
			if (ErrorDescription.IsEmpty())
				return FString::Printf(TEXT("%s"), *ErrorName.ToString());
			return FString::Printf(TEXT("%s: %s"), *ErrorName.ToString(), *ErrorDescription);
		}
	};
	
	TValueOrError<void, FReadyToStartMatchError> ReadyToStartMatchValue() const;
	
	UFUNCTION()
	void HandleUnitCatalogReady(bool bSuccess);
	UFUNCTION()
	void HandleUnitsReadyServer();
	
	void SpawnPlayerMainShip();
	void SpawnAIShips(int InAINumber);

	AWarpGameState* GetWarpGameState() const;

	FReadyToStartMatchError LastReadyToStartMatchError_;

	UWarpPlayfabContentSubSystem* Content_;
	
	bool bServerContentReady_ = false;
	
	bool bInitialUnitsSpawned = false;
	bool bMainPlayerSpawned = false;
	bool bAISpawned = false;
	int AINumber = 5;
};





