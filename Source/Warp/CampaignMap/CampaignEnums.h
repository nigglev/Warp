#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMapNodeType : uint8
{
	Undefined,
	Combat,
	Repair,
	Shop
};

UENUM(BlueprintType)
enum class EMapNodeState : uint8
{
	Completed,
	Captured,
	Available,
	Unaccessible
};

using FNodePosition = UE::Math::TIntVector2<uint8>;