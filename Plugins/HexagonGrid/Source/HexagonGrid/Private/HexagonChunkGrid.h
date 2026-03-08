// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
#include "HexMath.h"
#include "HexPathfainer.h"
#include "MoveParams.h"
#include "UObject/Object.h"
#include "HexagonChunkGrid.generated.h"

struct FHexCellDrawInfo;
class UFHexChunkManager;
struct FHullHexFootprint;

namespace HexMath
{
	struct FOffsetCoord;
}

class AHexGridISMActor;

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
class HEXAGONGRID_API UHexagonChunkGrid : public UObject
{
	GENERATED_BODY()
	
public:
	
	void OnChangeObserverPosition(const FVector& InNewPosition);
	
	void SetCellDrawing(const HexMath::FAxialCoord& InAxialCoord, const FHexCellDrawInfo& InCellInfo);
	
private:
	
	void CreateNewChunks(const FVector& InNewPosition);
	void CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition);
	
	int32 FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const;
	
	void OnCellChange(const HexMath::FAxialCoord& InAxialCoord, const FHexCellDrawInfo& InCellInfo);

	HexMath::FOffsetCoord CurrentChunkCoord_;
	
	UPROPERTY()
	TArray<FChunkData> ChunksList_;
	
	FHashTable ChunkIndexes_;
};