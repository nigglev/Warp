#pragma once

#include "CoreMinimal.h"
#include "UnitEnums.h"
#include "UnitSize.generated.h"


USTRUCT()
struct FUnitSize
{
	GENERATED_BODY()

	FUnitSize() = default;
	explicit FUnitSize(const EUnitSizeCategory In) : SizeCategory(In) {}
	
	static FUnitSize None()   { return FUnitSize(EUnitSizeCategory::None); }
	static FUnitSize Small()  { return FUnitSize(EUnitSizeCategory::Small); }
	static FUnitSize Medium() { return FUnitSize(EUnitSizeCategory::Medium); }
	static FUnitSize Big()    { return FUnitSize(EUnitSizeCategory::Big); }
	
	EUnitSizeCategory GetUnitSize() const {return SizeCategory;}
	FIntVector2 GetUnitTileLength() const
	{
		switch (SizeCategory)
		{
		case EUnitSizeCategory::None:	return FIntVector2(0,0);
		case EUnitSizeCategory::Small:  return FIntVector2(1,1);
		case EUnitSizeCategory::Medium: return FIntVector2(3,1);
		case EUnitSizeCategory::Big:    return FIntVector2(5,1);
		default:                        return FIntVector2(0,0);
		}
	}
	void SetUnitSize(const EUnitSizeCategory InSizeCategory) {SizeCategory = InSizeCategory;}
	
private:	
	UPROPERTY()
	EUnitSizeCategory SizeCategory = EUnitSizeCategory::None;
};
