#pragma once

#include "CoreMinimal.h"
#include "UnitEnums.h"
#include "Warp/ContentManagement/StaticDescriptions/EUnitSize.h"
#include "UnitSize.generated.h"


USTRUCT()
struct FUnitSize
{
	GENERATED_BODY()

	FUnitSize() = default;
	explicit FUnitSize(const EUnitSize In) : SizeCategory(In) {}
	
	static FUnitSize None()   { return FUnitSize(EUnitSize::None); }
	static FUnitSize Small()  { return FUnitSize(EUnitSize::Small); }
	static FUnitSize Medium() { return FUnitSize(EUnitSize::Medium); }
	static FUnitSize Big()    { return FUnitSize(EUnitSize::Big); }
	
	EUnitSize GetUnitSize() const {return SizeCategory;}
	FIntVector2 GetUnitTileLength() const
	{
		switch (SizeCategory)
		{
		case EUnitSize::None:	return FIntVector2(0,0);
		case EUnitSize::Small:  return FIntVector2(1,1);
		case EUnitSize::Medium: return FIntVector2(3,1);
		case EUnitSize::Big:    return FIntVector2(5,1);
		default:                return FIntVector2(0,0);
		}
	}
	void SetUnitSize(const EUnitSize InSizeCategory) {SizeCategory = InSizeCategory;}
	
private:	
	UPROPERTY()
	EUnitSize SizeCategory = EUnitSize::None;
};
