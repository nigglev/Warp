// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "WarpPlayfabContentSubSystem.generated.h"

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnitsLoaded);

UCLASS()
class WARP_API UWarpPlayfabContentSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& InCollection) override;
	
	void DownloadUnits();
	const FUnitDefinition* GetUnitDefinition(const FName& Id) const;
	
	UFUNCTION()
	bool AreUnitsLoaded() const { return bUnitsLoaded_; }
	
	FOnUnitsLoaded OnUnitsLoaded;

protected:
		
	void OnGetTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result);
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);
	
	PlayFabClientPtr ClientAPI_ = nullptr;
	TMap<FName, FUnitDefinition> Units_;

	bool bUnitsLoaded_ = false;

};
