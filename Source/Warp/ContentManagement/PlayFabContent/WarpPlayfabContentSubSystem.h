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
	bool IsContentLoaded() const { return bContentLoaded_; }
	void BroadcastContentIsLoaded(bool InbIsContentLoaded);
	
	FOnUnitsLoaded OnContentLoaded;

	template<typename Descr>
	const Descr& GetDescription(FName InDescriptionName)
	{
		const TUniquePtr<FBaseDescriptions>* BucketPtr = Descriptions_.Find(Descr::DescrName);
		checkf(BucketPtr && BucketPtr->IsValid(), TEXT("Descriptions bucket '%s' is missing or null."),
			*Descr::DescrName.ToString());

		const FBaseDescription* BaseDescr = (*BucketPtr)->Find(InDescriptionName);
		checkf(BaseDescr, TEXT("Description '%s' not found in bucket '%s'."),
			*InDescriptionName.ToString(), *Descr::DescrName.ToString());
		
		
		const Descr* D = static_cast<const Descr*>(BaseDescr);
		checkf(D, TEXT("Type mismatch for '%s' in bucket '%s'."),
			*InDescriptionName.ToString(), *Descr::DescrName.ToString());

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
	FString GetGameDataSourceFilePath() const;
	
	TMap<FName, TUniquePtr<FBaseDescriptions>> Descriptions_;
	
	UPROPERTY()
	UPlayFabLoginInfo* LoginInfo_ = nullptr;
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;
	
	UPROPERTY()
	UPlayFabStateManager* StateManager_ = nullptr;

	FLaunchContext LaunchContext_;
	
	FString VersionsFileName = TEXT("DescriptionVersions.json");

	bool bSaveDescriptionToPlayFabDone_ = false;
	bool bSaveVersionToPlayFabDone_ = false;
	bool bContentLoaded_ = false;
};

