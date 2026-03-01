// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "Warp/ContentManagement/FSM/ContentFSM.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpDescriptionBase.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameplayDescriptions.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameVersion.h"
#include "Warp/Utils/WarpUtils.h"
#include "WarpPlayfabContentSubSystem.generated.h"

class UPlayFabStateManager;
enum class EPlayFabContentStates : uint8;
/**
 * 
 */

UENUM()
enum class ERoleType : uint8
{
	NotSet = 0,
	Developer,
	Server,
	Client
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
	
	static const FGameplayDescription* GetGameplayDescription(const UObject* WorldContextObject); 
	
	bool SaveDescriptionToPlayFab(const FName& InDescriptionName);

	UContentFSM* GetContentFSM() const {return ContentFSM_;}
	ERoleType GetRoleType() const {return RoleType_;}
	FGameVersion GetGameVersionFromDataSource();

	bool WriteGameVersionToDataSource(const FGameVersion& InGameVersion);
	bool WriteDescriptionToDataSourceFromJson(const FString& InDescriptionName, const FString& InDescriptionJson);
	bool WriteDescriptionToDataSource(const FName& InDescriptionName);
	
	UFUNCTION()
	bool IsContentLoaded() const { return bContentLoaded_; }
	void OnContentCheckedAndLoaded(bool InContentLoaded);
	void OnSaveDone(bool InSaveSuccess);
	FOnUnitsLoaded OnContentLoaded;

	template<typename Descr>
	const Descr* GetDescription(FName InDescriptionName)
	{
		const TUniquePtr<FBaseDescriptions>* BucketPtr = Descriptions_.Find(Descr::DescrName);
		ensureMsgf(BucketPtr && BucketPtr->IsValid(), TEXT("Descriptions bucket '%s' is missing or null."),
			*Descr::DescrName.ToString());

		const FBaseDescription* BaseDescr = (*BucketPtr)->Find(InDescriptionName);
		ensureMsgf(BaseDescr, TEXT("Description '%s' not found in bucket '%s'."),
			*InDescriptionName.ToString(), *Descr::DescrName.ToString());
		
		
		const Descr* D = static_cast<const Descr*>(BaseDescr);
		ensureMsgf(D, TEXT("Type mismatch for '%s' in bucket '%s'."),
			*InDescriptionName.ToString(), *Descr::DescrName.ToString());

		return D;
	}
	
	template<typename Descr>
	const Descr* GetFirstDescription()
	{
		const TUniquePtr<FBaseDescriptions>* BucketPtr = Descriptions_.Find(Descr::DescrName);
		ensureMsgf(BucketPtr && BucketPtr->IsValid(), TEXT("Descriptions bucket '%s' is missing or null."),
			*Descr::DescrName.ToString());

		ensureMsgf((*BucketPtr)->Num() == 1, TEXT("Descriptions bucket '%s' must have one record."),
			*Descr::DescrName.ToString());
		
		const FBaseDescription* BaseDescr = (*BucketPtr)->At(0);
		const Descr* D = static_cast<const Descr*>(BaseDescr);
		ensureMsgf(D, TEXT("Type mismatch for the first record in bucket '%s'."),
			*Descr::DescrName.ToString());

		return D;
	}

protected:
	void InitializeDescriptions();
	bool ReadDescriptionsFromDataSource();
	
	bool WriteDescriptionsToDataSource();
	bool WriteDescriptionToDataSource_Internal(const FName& DescriptionName, FBaseDescriptions& Description);
	
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);
	FString GetGameDataSourceFilePath() const;

	FGameVersion UpdateGameVersion(FName InNewDescriptionName, int32 InNewDescriptionVersion);
	bool UpdateCachedGameData();

	static bool TryGetVersionFromJson(const FString& InJson, int32& OutVersion, FText* OutFailReason = nullptr);
	bool GetDescriptionNames(TArray<FName>& OutDescriptionNames, const FString& InFolderName) const;
	bool GetDescriptionNames(TArray<FName>& OutDescriptionNames, const TMap<FName, TUniquePtr<FBaseDescriptions>>& InDescriptionsMap) const;
	bool GetFileJson(const FString& InFolderName, const FString& InFileName, FString& OutJson) const;
	ERoleType GetCurrentRoleType() const;
	
	TMap<FName, TUniquePtr<FBaseDescriptions>> Descriptions_;

	UPROPERTY()
	UContentFSM* ContentFSM_ = nullptr;
	ERoleType RoleType_ = ERoleType::NotSet;
	
	bool bContentLoaded_ = false;
};


