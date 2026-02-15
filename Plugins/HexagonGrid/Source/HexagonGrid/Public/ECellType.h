#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ECellType : uint8
{
	Opened = 0,
	Captured,
	Closed	,
	MoveProjection,
	MAX_VALUE
};