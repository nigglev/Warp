#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ECellType : uint8
{
	Opened         = 0,
	MoveProjection,
	SuccessCaptured,
	MovingPath,
	Closed,
	DenyCapture,
	MAX
};