#pragma once

#include "CoreMinimal.h"
#include "PlayFab.h"

class UWarpPlayfabContentSubSystem;

class FDescriptionReaderBase
{
public:
	virtual ~FDescriptionReaderBase() = default;

	virtual FString GetName() const { return FString(); }
	virtual int32 GetVersion() const { return 0; }
	virtual bool ReadGameplaySource() { return false; }
	virtual bool SaveToPlayFab(const PlayFabServerPtr& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject) { return false; };
	virtual bool ReadFromPlayFab(const PlayFabServerPtr& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject) { return false; };
	virtual void UpdateDescriptionVersion() = 0;
	virtual void UpdateVersions(const FString& InDescriptionName, int32 InVersion) = 0;
};