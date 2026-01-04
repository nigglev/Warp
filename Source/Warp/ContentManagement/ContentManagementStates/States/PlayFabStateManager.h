// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EPlayFabContentStates.h"
#include "PlayFab.h"
#include "UObject/Object.h"
#include "Warp/ContentManagement/PlayFabContent/DescriptionReader.hpp"

#include "PlayFabStateManager.generated.h"

struct FDescriptionVersions;
class UWarpPlayfabContentSubSystem;
enum class EPlayFabContentStates : uint8;
/**
 * 
 */
struct WARP_API FPlayFabStateManagerData
{
	FString DescriptionName = FString("");
	virtual ~FPlayFabStateManagerData() {}
};

UCLASS()
class WARP_API UPlayFabStateManager : public UReaderObserver
{
	GENERATED_BODY()

public:
	void SetOwner(UWarpPlayfabContentSubSystem* InOwner);
	void SetState(EPlayFabContentStates InNewState, const FPlayFabStateManagerData* InData = nullptr);
	EPlayFabContentStates GetState() const {return CurrentState_; }

	virtual void OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess) override;
	virtual void OnDescriptionSavingResult(FDescriptionReaderBase* InDescription, bool InSuccess) override;

protected:
	void OnStateSet(const FPlayFabStateManagerData* InData = nullptr);

	void HandleStartLogin();
	void HandleProcessingLogin();
	void HandleLoginFailure();
	void HandleLoginSuccess();
	
	void HandleSavingDescriptions(const FString& InDescriptionName);
	void HandleDownloadingVersions();
	void HandleComparingVersions();
	void HandleGettingOutdatedContent();
	void HandleUpdatingContent();
	

	void HandleUpdateDone();

	void HandleFailure();
	void HandleFinished();

	UFUNCTION()
	void OnLoginResult(const bool InLoginRes);
	bool LoginToPlayFab();
	void GetOutdatedDescriptions(const FDescriptionVersions& LatestVersions, const FDescriptionVersions& CurrentVersions, TArray<TUniquePtr<FDescriptionReaderBase>>& OutOutdated);
	void Reset();
	
	
	static void StateChangedLog(EPlayFabContentStates InOldState, EPlayFabContentStates InNewState);

	UPROPERTY()
	UWarpPlayfabContentSubSystem* Owner_ = nullptr;
	EPlayFabContentStates CurrentState_ = EPlayFabContentStates::None;

	TUniquePtr<FUStructDescriptionReader<FDescriptionVersions>> Versions_;
	TUniquePtr<FUStructDescriptionReader<FDescriptionVersions>> CurrentVersions_;
	
	TArray<TUniquePtr<FDescriptionReaderBase>> DescriptionReaders_;
	TArray<TUniquePtr<FDescriptionReaderBase>> OutDatedDescriptionReaders_;
	TUniquePtr<FDescriptionReaderBase> CurrentDescriptionReader_;

	bool bVersionSaveSuccessful_ = false;
	bool bDescriptionSaveSuccessful_ = false;

	UPROPERTY()
	UPlayFabLoginInfo* LoginInfo_ = nullptr;
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;


	
};

