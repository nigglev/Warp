#pragma once

#include "CoreMinimal.h"


UENUM()
enum class EUnitAffiliation
{
	Neutral = 0	UMETA(DisplayName="Neutral"),
	Player		UMETA(DisplayName="Player"),
	Enemy		UMETA(DisplayName="Enemy"),
	Ally		UMETA(DisplayName="Ally"),
	Max			UMETA(DisplayName="MAX")
};