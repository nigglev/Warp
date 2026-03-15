#pragma once

#include "CoreMinimal.h"
#include "CannonParams.generated.h"

USTRUCT()
struct FCannonParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FIntVector2 LocalShift = FIntVector2(0, 0);
	
	UPROPERTY(EditAnywhere)
	float LocalRotation = 0;
	
	UPROPERTY(EditAnywhere)
	float SectorAngle = 60;
	
	UPROPERTY(EditAnywhere)
	int32 HexDistance = 5;
};