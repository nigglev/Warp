// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DescriptionReaderBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "WarpPlayfabContentSubSystem.generated.h"


class UPlayFabStateManager;
enum class EPlayFabContentStates : uint8;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FUnitDefinition
{
	GENERATED_BODY()
	
	FName UnitTypeName;
	FString UnitSize;
	int32 UnitSpeed = 0;
	int32 UnitMaxAP = 0;
};

DECLARE_MULTICAST_DELEGATE(FOnUnitsLoaded);

UCLASS()
class WARP_API UWarpPlayfabContentSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UWarpPlayfabContentSubSystem();
	
	static UWarpPlayfabContentSubSystem* Get(const UObject* WorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& InCollection) override;

	void LoginToPlayFab();
	void SaveDescriptionToPlayFab(const FString& InDescriptionName);
	void UpdateContent();
	
	bool IsClient() const;
	void BroadcastContentIsLoaded(bool InbIsContentLoaded);
	
	UFUNCTION()
	bool IsClientDataLoaded() const { return bUnitsLoaded_; }
	
	FOnUnitsLoaded OnUnitsLoaded;

protected:
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);

	bool bUnitsLoaded_ = false;

	UPROPERTY()
	UPlayFabStateManager* StateManager_ = nullptr;

};
