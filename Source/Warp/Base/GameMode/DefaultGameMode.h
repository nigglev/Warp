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

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	bool StartBattle();
	
protected:

	virtual void OnMatchStateSet() override;
	
	virtual void HandleMatchHasLoading();
	virtual bool CheckLoading();

	virtual void HandleUnitCreation();
	virtual bool CheckUnitCreation();
	
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchIsWaitingToStart() override;

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

	TValueOrError<void, FReadyToStartMatchError> PlayersAndServerLoadValue() const;
	
	UFUNCTION()
	void CheckServerContentLoading();

	AWarpGameState* GetWarpGameState() const;

	UPROPERTY(EditDefaultsOnly, Category="Data")
	UDataTable* UnitsTable_ = nullptr;

	FReadyToStartMatchError LastPlayersAndServerLoadError_;
	
	bool bServerContentReady_ = false;
	bool bUnitsCreated_ = false;
};





