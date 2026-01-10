// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexMath.h"
#include "UObject/Object.h"
#include "HexagonChunkGrid.generated.h"

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
		
	static uint32 CalcKey(const HexMath::FOffsetCoord& InChunkIndex) { return HashCombine( GetTypeHash(InChunkIndex.Right), GetTypeHash(InChunkIndex.Up)); } 
		
	FChunkData() = default;
	FChunkData(const HexMath::FOffsetCoord& InChunkIndex, AHexGridISMActor* InChunkActor) 
		: ChunkIndex(InChunkIndex), ChunkActor(InChunkActor)
	{
		Key = CalcKey(InChunkIndex);
	}
};

/**
 * 
 */
UCLASS()
class HEXAGONGRID_API UHexagonChunkGrid : public UObject
{
	GENERATED_BODY()
	
public:
	void OnChangeObserverPosition(const FVector& InNewPosition);
	
	void SelectCell(const FVector& InPosition);
	
private:
	static TOptional<HexMath::FOffsetCoord> WorldToChunkCoord(const FVector& InWorldPoint);
	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	
	struct FHexGridActorCDODataCache
	{
		float HexSize = 0;
		uint32 NumColsRows = 0;
		uint32 BuildChunkAround = 0;
		TSubclassOf<AHexGridISMActor> HexGridActorClass_ = nullptr;
	};
	static TOptional<FHexGridActorCDODataCache> GetHexGridActorCDODataCache();
	
	void CreateNewChunks(const FVector& InNewPosition);
	
	int32 FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const;
	
	void SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows);

	HexMath::FOffsetCoord CurrentChunkCoord_;
	
	UPROPERTY()
	TArray<FChunkData> ChunksList_;
	
	FHashTable ChunkIndexes_;
};
