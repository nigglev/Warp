#pragma once
#include "CoreMinimal.h"
#include "ECellType.h"

class FCellLayers
{
public:
	FCellLayers()
	{
		for (uint8 i = 0; i < static_cast<uint8>(ECellType::MAX); i++)
		{
			Levels_[i] = 0;
		}
	}
	uint8 MaxIndex() const
	{
		for (uint8 i = static_cast<uint8>(ECellType::MAX) - 1; i > 0; i--)
		{
			if (Levels_[i] > 0)
			{
				return i;
			}
		}
		return 0;
	}
	
	ECellType GetCellType() const { return static_cast<ECellType>(MaxIndex()); }
	
	void SetCellType(ECellType InCellType, float InLevel)
	{
		uint8 Index = static_cast<uint8>(InCellType);
		Levels_[Index] = InLevel;
	}
	
	float GetLevel(ECellType InCellType) const
	{
		uint8 Index = static_cast<uint8>(InCellType);
		return Levels_[Index];
	}
	
	float GetMaxLevel() const
	{
		return Levels_[MaxIndex()];
	}
	
private:
	float Levels_[static_cast<uint8>(ECellType::MAX)];
};
