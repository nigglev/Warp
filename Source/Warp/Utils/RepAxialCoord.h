#pragma once
#include "HexMath.h"
#include "RepAxialCoord.generated.h"

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
	
	// неявно wrapper -> native
	//operator HexMath::FAxialCoord() const { return HexMath::FAxialCoord{ Q, R }; }
};
