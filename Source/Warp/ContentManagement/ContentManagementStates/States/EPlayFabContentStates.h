#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPlayFabContentStates : uint8
{
	None = 0	UMETA(DisplayName="None"),
	StartLogin = 1 UMETA(DisplayName="StartLogin"),
	StartLoginEditor = 2 UMETA(DisplayName="StartLoginEditor"),
	ProcessingLogin = 3 UMETA(DisplayName="ProcessingLogin"),
	LoginFailure = 4 UMETA(DisplayName="LoginFailure"),
	LoginSuccess = 5 UMETA(DisplayName="LoginSuccess"),
	
	SaveDescriptions = 6 UMETA(DisplayName="SaveDescriptions"),
	
	DownloadVersions = 7 UMETA(DisplayName="DownloadingVersions"),
	CompareVersions = 8 UMETA(DisplayName="CheckingVersions"),
	GettingOutdatedContent = 9 UMETA(DisplayName="GettingOutdatedContent"),
	UpdatingContent = 10 UMETA(DisplayName="UpdatingContent"),
	
	UpdateDone = 11 UMETA(DisplayName="UpdatingDone"),
	UpdatePending= 12 UMETA(DisplayName="UpdateFailed"),

	Failure = 13 UMETA(DisplayName="Failure"),
	Finished = 14 UMETA(DisplayName="Finished"),
};