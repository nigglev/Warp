#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMapNodeType : uint8
{
	Combat,
	Repair,
	Shop
};

UENUM(BlueprintType)
enum class EMapNodeState : uint8
{
	Locked,
	Available,
	Completed
};