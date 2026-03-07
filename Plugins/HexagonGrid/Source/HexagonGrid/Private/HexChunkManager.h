#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
#include "HexMath.h"
#include "UObject/Object.h"
#include "HexChunkManager.generated.h"

class AHexGridISMActor;
class UHexagonChunkGrid;

USTRUCT()
struct FChunkData
{
	GENERATED_BODY()
	
	uint32 Key = 0;
	
	HexMath::FOffsetCoord ChunkIndex;
	
	UPROPERTY()
	AHexGridISMActor* ChunkActor = nullptr;
		
	static uint32 CalcKey(const HexMath::FOffsetCoord& InChunkIndex) { return HashCombine( GetTypeHash(InChunkIndex.Col), GetTypeHash(InChunkIndex.Row)); } 
		
	FChunkData() = default;
	FChunkData(const HexMath::FOffsetCoord& InChunkIndex, AHexGridISMActor* InChunkActor) 
		: ChunkIndex(InChunkIndex), ChunkActor(InChunkActor)
	{
		Key = CalcKey(InChunkIndex);
	}
};

UCLASS()
class UFHexChunkManager: public UObject
{
	GENERATED_BODY()
	
public:
	
	UHexagonChunkGrid* GetOwner() const;
	
	void CreateNewChunks(const FVector& InNewPosition);
	void CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition);
	
	int32 FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const;
	
	void SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel);
	void SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel);
	
	void SetCellType(const FVector& InPosition, ECellType InCellType);
private:
	
	TSet<HexMath::FAxialCoord> Obstacles_;
	
	HexMath::FOffsetCoord CurrentChunkCoord_;
	
	UPROPERTY()
	TArray<FChunkData> ChunksList_;
	
	FHashTable ChunkIndexes_;
};
