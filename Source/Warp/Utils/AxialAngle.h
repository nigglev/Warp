#pragma once
#include "AxialAngle.generated.h"

USTRUCT()
struct FAxialAngle
{
	GENERATED_BODY()

	UPROPERTY()
	int8 R = 0;

	FAxialAngle() = default;
	
	static int8 GetDirectionAngle(float Yaw) { return FMath::RoundToInt(Yaw / 60.f); }
	
	void SetByYaw(float Yaw)
	{
		R = GetDirectionAngle(Yaw);
	}
		
	float GetYaw() const { return R * 60.f; }
	
	FString ToString() const { return FString::Printf(TEXT("%d"), R); }
};
