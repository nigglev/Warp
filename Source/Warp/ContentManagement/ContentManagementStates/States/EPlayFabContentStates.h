#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPlayFabContentStates : uint8
{
	None = 0	UMETA(DisplayName="None"),
	CheckUpdate = 1 UMETA(DisplayName="CheckUpdate"),
	DownloadVersions = 2 UMETA(DisplayName="DownloadingVersions"),
	CompareVersions = 3 UMETA(DisplayName="CheckingVersions"),
	GettingOutdatedContent = 4 UMETA(DisplayName="GettingOutdatedContent"),
	UpdatingContent = 5 UMETA(DisplayName="UpdatingContent"),
	
	UpdateDone = 6 UMETA(DisplayName="UpdatingDone"),
	UpdateFailed = 7 UMETA(DisplayName="UpdateFailed"),

	Finished = 8 UMETA(DisplayName="Finished"),

};