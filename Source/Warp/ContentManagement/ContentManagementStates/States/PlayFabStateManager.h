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
	TUniquePtr<FUStructDescriptionReader<FDescriptionVersions>> Versions;
	TUniquePtr<FUStructDescriptionReader<FDescriptionVersions>> OldVersions;
	
	TArray<TUniquePtr<FDescriptionReaderBase>> DescriptionReaders;
	TUniquePtr<FDescriptionReaderBase> CurrentDescriptionReader;
	virtual ~FPlayFabStateManagerData() {}
};

UCLASS()
class WARP_API UPlayFabStateManager : public UReaderObserver
{
	GENERATED_BODY()

public:
	void Start();
	void SetClientAPI(const PlayFabClientPtr& InClientAPI) {ClientAPI_ = InClientAPI;};
	EPlayFabContentStates GetState() const {return CurrentState_; }

	virtual void OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess) override;

protected:
	void SetState(EPlayFabContentStates InNewState, FPlayFabStateManagerData& InData);
	void OnStateSet(FPlayFabStateManagerData& InData);

	void HandleCheckUpdate(FPlayFabStateManagerData& InData);
	void HandleDownloadingVersions(FPlayFabStateManagerData& InData);
	void HandleComparingVersions(FPlayFabStateManagerData& InData);
	void HandleGettingOutdatedContent(FPlayFabStateManagerData& InData);
	void HandleUpdatingContent(FPlayFabStateManagerData& InData);
	void HandleUpdateDone(FPlayFabStateManagerData& InData);
	void HandleUpdateFailed(FPlayFabStateManagerData& InData);
	void HandleFinished(FPlayFabStateManagerData& InData);

	
	void GetOutdatedDescriptions(const FDescriptionVersions& LatestVersions, const FDescriptionVersions& CurrentVersions, TArray<TUniquePtr<FDescriptionReaderBase>>& OutOutdated);
	static void StateChangedLog(EPlayFabContentStates InOldState, EPlayFabContentStates InNewState);

	FPlayFabStateManagerData ContextData_;
	EPlayFabContentStates CurrentState_ = EPlayFabContentStates::None;
	PlayFabClientPtr ClientAPI_ = nullptr;
};

