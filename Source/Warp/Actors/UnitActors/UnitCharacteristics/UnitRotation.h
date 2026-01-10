#pragma once

#include "CoreMinimal.h"
#include "UnitRotation.generated.h"

USTRUCT()
struct FUnitRotation
{
	GENERATED_BODY()

	FUnitRotation() = default;
	explicit FUnitRotation(const EUnitRotation In) : UnitRotation(In) {}
	
	static FUnitRotation Max()   { return FUnitRotation(EUnitRotation::Max); }
	static FUnitRotation Rot0()  { return FUnitRotation(EUnitRotation::Rot_0); }
	static FUnitRotation Rot45() { return FUnitRotation(EUnitRotation::Rot_45); }
	static FUnitRotation Rot90()    { return FUnitRotation(EUnitRotation::Rot_90); }
	static FUnitRotation Rot35()   { return FUnitRotation(EUnitRotation::Rot_135); }
	static FUnitRotation Rot180()  { return FUnitRotation(EUnitRotation::Rot_180); }
	static FUnitRotation Rot225() { return FUnitRotation(EUnitRotation::Rot_225); }
	static FUnitRotation Rot270()    { return FUnitRotation(EUnitRotation::Rot_270); }
	static FUnitRotation Rot315()    { return FUnitRotation(EUnitRotation::Rot_315); }
	
	void RotateClockwise()
	{
		constexpr uint8 Count = static_cast<uint8>(EUnitRotation::Max);
		const uint8 Cur   = static_cast<uint8>(UnitRotation) % Count;
		UnitRotation      = static_cast<EUnitRotation>((Cur + 1) % Count);
	}

	void RotateCounterClockwise()
	{
		constexpr uint8 Count = static_cast<uint8>(EUnitRotation::Max);
		const uint8 Cur   = static_cast<uint8>(UnitRotation) % Count;
		UnitRotation      = static_cast<EUnitRotation>((Cur + Count - 1) % Count);
	}
		
	EUnitRotation GetUnitRotation() const {return UnitRotation;}
	float GetUnitFRotation () const
	{
		const uint8 i = static_cast<uint8>(UnitRotation);
		return (i < static_cast<uint8>(EUnitRotation::Max)) ? i * 45.f : 0.f;
	}

	void SetDefaultUnitRotation() {UnitRotation = EUnitRotation::Rot_0;}
	void SetRandomRotation() {UnitRotation = static_cast<EUnitRotation>(FMath::RandRange(0, static_cast<uint8>(EUnitRotation::Max) - 1));}
	void SetUnitRotation(const EUnitRotation InUnitRotation) {UnitRotation = InUnitRotation;}
	void SetUnitRotation(const float InUnitRotation)
	{
		float Norm = FMath::Fmod(InUnitRotation, 360.f);
		if (Norm < 0.f)
			Norm += 360.f;
		const int32 Step = FMath::RoundToInt(Norm / 45.f) % 8;
		UnitRotation = static_cast<EUnitRotation>(1 + Step);
	}
	
private:	
	UPROPERTY()
	EUnitRotation UnitRotation = EUnitRotation::Max;
};
