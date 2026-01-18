#pragma once

#include "CoreMinimal.h"
#include "PlayFab.h"
#include "WarpPlayFabContentExtension.h"
#include "DescriptionReaderBase.generated.h"


struct FBaseDescriptions;
class UWarpPlayFabContentManager;
class UPlayFabStateManager;
class UWarpPlayfabContentSubSystem;

class FDescriptionReaderBase;

using FAnyPlayFabPtr = TVariant<PlayFabServerPtr, PlayFabClientPtr>;

UCLASS()
class UReaderObserver : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess) PURE_VIRTUAL();
};

class FDescriptionReaderBase
{
public:
	virtual ~FDescriptionReaderBase() = default;

	virtual FString GetName() const { return FString(); }
	virtual int32 GetVersion() const { return 0; }
	virtual bool ReadGameplaySource(const FAnyPlayFabPtr& InApi) { return false; }
	virtual bool WriteGameplaySource(const FAnyPlayFabPtr& InApi) { return false; }
	virtual bool ReadFromPlayFab(const FAnyPlayFabPtr& InPlayFabAPI, UReaderObserver* InUserObject) { return false; };
};