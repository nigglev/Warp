// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DescriptionReaderBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"
#include "Warp/Utils/WarpUtils.h"
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
	virtual void Initialize(FSubsystemCollectionBase& InCollection) override;
	
	static UWarpPlayfabContentSubSystem* Get(const UObject* WorldContextObject);
	
	bool SaveDescriptionToPlayFab(const FName& InDescriptionName);
	void OnDescriptionSavingResult(bool bSucceeded);
	
	UFUNCTION()
	bool IsClientDataLoaded() const { return bUnitsLoaded_; }
	void BroadcastContentIsLoaded(bool InbIsContentLoaded);
	
	FOnUnitsLoaded OnUnitsLoaded;

	template<typename Descr>
	const Descr& GetDescr(FName InDescrName)
	{
		FBaseDescriptions& Descriptions = Descriptions_.FindOrAdd(Descr::DescrName);
		
		const FBaseDescription* BaseDescr = Descriptions.Find(InDescrName);

		const Descr* D = static_cast<const Descr*>(BaseDescr);
		ensure(D);
		return *D;
	}

protected:
	bool LoginToPlayFab();

	bool ReadDescriptionsFromDataSource();
	
	bool WriteDescriptionsToDataSource();
	bool WriteDescriptionToDataSource(const FName& InDescriptionName);
	bool WriteDescriptionToDataSource_Internal(const FName& DescriptionName, FBaseDescriptions& Description);

	FDescriptionVersions CreateVersions();
	bool WriteVersionsToDataSource(const FDescriptionVersions& InVersions, FString& OutJsonString);
	bool SaveVersionsToPlayFab();
	
	
	UFUNCTION()
	void OnLoginResult(const bool InLoginRes);
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);

	bool IsClient() const;
	bool IsClientOnly() const;
	bool IsClientEditor() const;
	
	bool IsServerOnly() const;
	bool IsServerEditor() const;
	
	FString GetGameDataSourceFilePath() const;
	
	TMap<FName, TUniquePtr<FBaseDescriptions>> Descriptions_;
	UPROPERTY()
	UPlayFabStateManager* StateManager_ = nullptr;
	
	UPROPERTY()
	UPlayFabLoginInfo* LoginInfo_ = nullptr;
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;

	FString VersionsFileName = TEXT("DescriptionVersions.json");

	bool bSaveDescriptionToPlayFabDone_ = false;
	bool bSaveVersionToPlayFabDone_ = false;
	bool bUnitsLoaded_ = false;
};

