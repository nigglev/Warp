#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPlayFabContentStates : uint8
{
	None = 0	UMETA(DisplayName="None"),
	StartLogin = 1 UMETA(DisplayName="StartLogin"),
	ProcessingLogin = 2 UMETA(DisplayName="ProcessingLogin"),
	LoginFailure = 3 UMETA(DisplayName="LoginFailure"),
	LoginSuccess = 4 UMETA(DisplayName="LoginSuccess"),
	
	SaveDescriptions = 5 UMETA(DisplayName="SaveDescriptions"),
	
	DownloadVersions = 6 UMETA(DisplayName="DownloadingVersions"),
	CompareVersions = 7 UMETA(DisplayName="CheckingVersions"),
	GettingOutdatedContent = 8 UMETA(DisplayName="GettingOutdatedContent"),
	UpdatingContent = 9 UMETA(DisplayName="UpdatingContent"),
	
	UpdateDone = 10 UMETA(DisplayName="UpdatingDone"),
	UpdatePending= 11 UMETA(DisplayName="UpdateFailed"),

	Failure = 12 UMETA(DisplayName="Failure"),
	Finished = 13 UMETA(DisplayName="Finished"),
};