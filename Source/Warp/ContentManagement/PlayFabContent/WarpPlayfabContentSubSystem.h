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

DECLARE_MULTICAST_DELEGATE(FOnUnitsLoaded);

UCLASS()
class WARP_API UWarpPlayfabContentSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	static UWarpPlayfabContentSubSystem* Get(const UObject* WorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& InCollection) override;
	
	void SetPlayFabId(const FString& InPlayFabId) { PlayFabId_ = InPlayFabId; }
	void SetEntityToken(const FString& InEntityToken, const FDateTime& InExpiration) { EntityToken_ = InEntityToken; EntityTokenExpiration_ = InExpiration; }
	void SetSessionTicket(const FString& InSessionTicket) { SessionTicket_ = InSessionTicket; }
	
	FString GetPlayFabId() const { return PlayFabId_; }
	FString GetEntityToken() const { return EntityToken_; }
	FDateTime GetEntityTokenExpiration() const { return EntityTokenExpiration_; }
	FString GetSessionTicket() const { return SessionTicket_; }
	
	void DownloadUnits();
	const FUnitDefinition* GetUnitDefinition(const FName& Id) const;
	
	UFUNCTION()
	bool IsClientDataLoaded() const { return bUnitsLoaded_; }
	
	FOnUnitsLoaded OnUnitsLoaded;

protected:
	
	void OnGetTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result);
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);
	
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;
	
	TMap<FName, FUnitDefinition> Units_;

	bool bUnitsLoaded_ = false;
	
	FString PlayFabId_;
	FString EntityToken_;
	FDateTime EntityTokenExpiration_;
	FString SessionTicket_;
};
