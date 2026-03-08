#pragma once
#include "HexMath.h"
#include "RepAxialCoord.generated.h"

USTRUCT(BlueprintType)
struct FAxialAngle
{
	GENERATED_BODY()

	UPROPERTY()
	int8 R = 0;

	FAxialAngle() = default;
	explicit FAxialAngle(int8 InR) : R(InR) {}
	
	auto operator<=>(const FAxialAngle& AxialCoord) const = default;
	
	static int8 GetDirectionAngle(float Yaw) { return FMath::RoundToInt(Yaw / 60.f); }
	
	void SetByYaw(float Yaw)
	{
		R = GetDirectionAngle(Yaw);
	}
	
	static float GetYaw(int8 InR) { return InR * 60.f; }
	
	float GetYaw() const { return R * 60.f; }
	
	FString ToString() const { return FString::Printf(TEXT("%d"), R); }
};

USTRUCT(BlueprintType)
struct FRepAxialCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Q = INT32_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 R = INT32_MAX;

	FRepAxialCoord() = default;

	FRepAxialCoord(const HexMath::FAxialCoord& In) : Q(In.Q), R(In.R) {}

	FORCEINLINE HexMath::FAxialCoord ToNative() const
	{
		return HexMath::FAxialCoord{ Q, R };
	}

	auto operator<=>(const FRepAxialCoord& AxialCoord) const = default;

	FString ToString() const { return FString::Printf(TEXT("(%d, %d)"), Q, R); }
};

USTRUCT(BlueprintType)
struct FAxialTransform
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRepAxialCoord Position;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAxialAngle Rotation;

	FAxialTransform() = default;
	explicit FAxialTransform(const FRepAxialCoord& InPosition, const FAxialAngle& InRotation) : Position(InPosition), Rotation(InRotation) {}
	explicit FAxialTransform(const HexMath::FAxialCoord& InPosition, const FAxialAngle& InRotation) : Position(InPosition), Rotation(InRotation) {}
	explicit FAxialTransform(const HexMath::FAxialCoord& InPosition, int8 InRotation) : Position(InPosition), Rotation(InRotation) {}
	
	auto operator<=>(const FAxialTransform& AxialCoord) const = default;
	
	FString ToString() const { return FString::Printf(TEXT("Pos: %s; Rot: %s"), *Position.ToString(), *Rotation.ToString()); }
};