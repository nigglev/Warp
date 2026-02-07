// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "Warp/CampaignMap/CampaignEnums.h"
#include "DefaultGameMode.generated.h"

class UUnitActorFactory;
class ABaseUnitActor;
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
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	bool StartBattle();
	
	EMapNodeType GetMapNode() const { return MapNodeType_; }
	void ReturnToCampaignMap();

protected:
	
	UPROPERTY(EditAnywhere, Category="Map") FName CampaignMap;

	virtual void OnMatchStateSet() override;

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
	
	
	virtual void HandleMatchLoading();
	virtual void HandleUnitCreation();
	
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchIsWaitingToStart() override;
	
	void CheckServerContentLoaded();
	virtual bool CheckPlayersAndServerContentLoaded();
	TValueOrError<void, FReadyToStartMatchError> PlayersAndServerContentLoadValue() const;
	
	virtual bool CheckServerUnitCreation();
	virtual bool CheckPlayersAndServerUnitCreation();
	TValueOrError<void, FReadyToStartMatchError> PlayersAndServerUnitCreationValue() const;
	
	AWarpGameState* GetWarpGameState() const;
	UUnitActorFactory* CreateUnitsFactory();

	UPROPERTY(EditDefaultsOnly, Category="Data")
	UDataTable* UnitsTable_ = nullptr;

	FReadyToStartMatchError LastPlayersAndServerLoadError_;
	
	bool bServerContentReady_ = false;
	bool bUnitsCreated_ = false;

	EMapNodeType MapNodeType_ = EMapNodeType::Undefined;
	FNodePosition NodePosition_ = FNodePosition::ZeroValue;
};





