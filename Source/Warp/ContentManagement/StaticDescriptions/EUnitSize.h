#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EUnitSize : uint8
{
	None = 0	UMETA(DisplayName="None"),
	Small		UMETA(DisplayName="Small (1×1)"),
	Medium		UMETA(DisplayName="Medium (2×1)"),
	Big			UMETA(DisplayName="Big (3×1)")
};