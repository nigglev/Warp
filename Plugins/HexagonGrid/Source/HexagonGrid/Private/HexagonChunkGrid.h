// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CellLayers.h"
#include "ECellType.h"
#include "HexMath.h"
#include "HexPathfainer.h"
#include "MoveParams.h"
#include "UObject/Object.h"
#include "HexagonChunkGrid.generated.h"

class UFHexChunkManager;
struct FHullHexFootprint;

namespace HexMath
{
	struct FOffsetCoord;
}

class AHexGridISMActor;

/**
 * 
 */
UCLASS()
class HEXAGONGRID_API UHexagonChunkGrid : public UObject
{
	GENERATED_BODY()
	
public:
	UHexagonChunkGrid();
	
	void OnChangeObserverPosition(const FVector& InNewPosition);
	
	void SetCellType(const FVector& InPosition, ECellType InCellType);
	
	void CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull);
	void ReleaseCells(uint32 InId);
	
	void SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, 
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath = nullptr);
	
	void RemoveInfluence(uint32 InId);
	
	void FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes);
	
	void DropPathSelections();
	
private:
	
	void CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition);

	void ClearCells(uint32 InId, ECellType InCellType);
	
	void SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel);
	
	TMap<HexMath::FAxialCoord, FCellLayers> SelectStatus_;
	
	TArray<HexMath::FPathNode> PFCells_;
	
	TMap<uint32, TArray<HexMath::FAxialCoord>> InfluencedCells_;
	TMap<uint32, TArray<HexMath::FAxialCoord>> CapturedCells_;
	
	TSet<HexMath::FAxialCoord> Obstacles_;
	
	UPROPERTY()
	UFHexChunkManager* ChunkManager_;
	
	friend UFHexChunkManager;
};