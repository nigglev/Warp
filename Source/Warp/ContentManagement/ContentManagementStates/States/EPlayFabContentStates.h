#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPlayFabContentStates : uint8
{
	None = 0	UMETA(DisplayName="None"),
	Login = 1 UMETA(DisplayName="Login"),
	LoginFailure = 2 UMETA(DisplayName="LoginFailure"),
	LoginSuccess = 3 UMETA(DisplayName="LoginSuccess"),
	
	SaveDescriptions = 5 UMETA(DisplayName="SaveDescriptions"),
	
	DownloadVersions = 6 UMETA(DisplayName="DownloadingVersions"),
	CompareVersions = 7 UMETA(DisplayName="CheckingVersions"),
	UpdatingContent = 8 UMETA(DisplayName="UpdatingContent"),
	
	UpdateDone = 9 UMETA(DisplayName="UpdatingDone"),
	UpdatePending= 10 UMETA(DisplayName="UpdateFailed"),

	Failure = 11 UMETA(DisplayName="Failure"),
	Finished = 12 UMETA(DisplayName="Finished"),
};