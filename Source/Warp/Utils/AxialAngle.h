#pragma once
#include "AxialAngle.generated.h"

USTRUCT()
struct FAxialAngle
{
	GENERATED_BODY()

	UPROPERTY()
	int8 R = 0;

	FAxialAngle() = default;
	
	void SetByYaw(float Yaw)
	{
		R = FMath::RoundToInt(Yaw / 60.f);
	}
	
	float GetYaw() const { return R * 60.f; }
};
