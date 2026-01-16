// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DescriptionReaderBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"
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

	
	void SaveDescriptionToPlayFab(const FString& InDescriptionName);
	
	bool IsClient() const;
	void BroadcastContentIsLoaded(bool InbIsContentLoaded);
	
	UFUNCTION()
	bool IsClientDataLoaded() const { return bUnitsLoaded_; }
	
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

	bool ReadDescriptions();

	UFUNCTION()
	void OnLoginResult(const bool InLoginRes);
	void OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult);

	UPROPERTY()
	UPlayFabLoginInfo* LoginInfo_ = nullptr;
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;
	
	TMap<FName, TUniquePtr<FBaseDescriptions>> Descriptions_;
	
	bool bUnitsLoaded_ = false;

	UPROPERTY()
	UPlayFabStateManager* StateManager_ = nullptr;
};

