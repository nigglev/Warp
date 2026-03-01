#pragma once

#include "CoreMinimal.h"
#include "MoveParams.generated.h"

USTRUCT()
struct FMoveParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float MaxDistance = 50;
	
	UPROPERTY(EditAnywhere)
	float MoveCost = 10;
	
	UPROPERTY(EditAnywhere)
	float RotationCost = 20;
	
	FString ToString() const
	{
		return FString::Printf(TEXT("MaxDistance: %f, MoveCost: %f, RotationCost: %f"), MaxDistance, MoveCost, RotationCost);
	}
};