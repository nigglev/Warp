#pragma once

#include "CoreMinimal.h"
#include "PlayFab.h"

class UWarpPlayfabContentSubSystem;

class FDescriptionReaderBase
{
public:
	virtual ~FDescriptionReaderBase() = default;

	virtual FString GetName() const { return FString(); }
	virtual bool ReadGameplaySource() { return false; }
	virtual bool SaveToPlayFab(const PlayFabServerPtr& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject) { return false; };
};