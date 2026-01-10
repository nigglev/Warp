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

UENUM()
enum class EUnitRotation : uint8
{
	Rot_0 = 0	UMETA(DisplayName="0°"),
	Rot_45		UMETA(DisplayName="45°"),
	Rot_90		UMETA(DisplayName="90°"),
	Rot_135		UMETA(DisplayName="135°"),
	Rot_180		UMETA(DisplayName="180°"),
	Rot_225		UMETA(DisplayName="225°"),
	Rot_270		UMETA(DisplayName="270°"),
	Rot_315		UMETA(DisplayName="315°"),
	Max			UMETA(DisplayName="MAX")
};

UENUM()
enum class EUnitSizeCategory : uint8
{
	None = 0	UMETA(DisplayName="None"),
	Small		UMETA(DisplayName="Small (1×1)"),
	Medium		UMETA(DisplayName="Medium (2×1)"),
	Big			UMETA(DisplayName="Big (3×1)")
};
