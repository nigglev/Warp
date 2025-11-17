// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Warp/UnitStaticData/PlayFabUnitTypes.h"
#include "UnitDataSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitCatalogReady, bool, bSuccess);

struct FUnitRecordDTO;


UCLASS(Config=Game)
class WARP_API UUnitDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void SetUnitCatalog_Client(const TArray<FUnitRecordDTO>& InUnitDTO);
	bool IsUnitCatalogReady() const { return bUnitCatalogReady; }

	const TMap<FName, FUnitRecord>& GetUnitData() const { return RemoteUnits; }
	FUnitRecord GetPlayerMainShipRecord() const;
	FUnitRecord GetCorvetteRecord() const;
	FSoftObjectPath GetUnitMeshObjectPath(const FName& InUnitTypeName) const;
	
	FOnUnitCatalogReady OnUnitCatalogReady;
	
protected:
	void StartAuthAndLoadUnits();
	void FetchUnitsFromCatalog(const FString& InEntityToken, const FString& InContinuation  = TEXT(""));
	void ParseUnitsFromSearchResponse(const FString& InResponseJson);
	bool ReadPlayFabSecretFromFile(FString& OutSecret) const;
	
	UPROPERTY(EditAnywhere, Config, Category="PlayFab")
	FString PlayFabTitleId = TEXT("A88F9");

	bool bUnitCatalogReady = false;
	TMap<FName, FUnitRecord> RemoteUnits;
};

