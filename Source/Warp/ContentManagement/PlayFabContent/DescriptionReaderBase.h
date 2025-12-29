#pragma once

#include "CoreMinimal.h"
#include "PlayFab.h"
#include "DescriptionReaderBase.generated.h"

class UPlayFabStateManager;
class UWarpPlayfabContentSubSystem;

class FDescriptionReaderBase;

UCLASS()
class UReaderObserver : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess) PURE_VIRTUAL();
	virtual void OnDescriptionSavingResult(FDescriptionReaderBase* InDescription, bool InSuccess) PURE_VIRTUAL();
};

class FDescriptionReaderBase
{
public:
	virtual ~FDescriptionReaderBase() = default;

	virtual FString GetName() const { return FString(); }
	virtual int32 GetVersion() const { return 0; }
	virtual bool ReadGameplaySource() { return false; }
	virtual bool WriteGameplaySource() { return false; }
	virtual bool SaveToPlayFab(const PlayFabServerPtr& InPlayFabAPI, UReaderObserver* InUserObject) { return false; };
	virtual bool ReadFromPlayFab(const PlayFabServerPtr& InPlayFabAPI, UReaderObserver* InUserObject) { return false; };
	virtual void UpdateDescriptionVersion() = 0;
};