// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
#include "GraphAStar.h"
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

class FSearchNode : public TSharedFromThis<FSearchNode>
{
public:
	// virtual ~FSearchNode() { }
	// virtual void GetChildren(TArray<TSharedPtr<FSearchNode>>& OutChildren) { }
	// virtual ESearchNodeType GetType() const = 0;
	// virtual FString GetText() const = 0;
	// virtual FString GetObjectPath() const = 0;
	//
	// float GetTotalScore() const { return TotalScore; }
	// float GetMaxScore() const { return MaxScore; }

protected:
	float TotalScore = 0;
	float MaxScore = 0;
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
	void SetCellType(const FVector& InPosition, ECellType InCellType);
	
	void SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, uint32 InHexDistance, 
		TArray<HexMath::FAxialCoord>* OutPath = nullptr);
	
	void RemoveInfluence(uint32 InId);
	
	void FindPath(const HexMath::FAxialCoord& InStart, const HexMath::FAxialCoord& InEnd, 
		TArray<HexMath::FAxialCoord>& OutPath, bool InLog = false) const;
	
	void SelectedFindPath(const HexMath::FAxialCoord& InStart, const HexMath::FAxialCoord& InEnd, 
		TArray<HexMath::FAxialCoord>& OutPath);
	
private:
	static TOptional<HexMath::FOffsetCoord> WorldToChunkCoord(const FVector& InWorldPoint);
	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	
	struct FHexGridActorCDODataCache
	{
		float HexSize = 0;
		int32 NumColsRows = 0; //размеры чанка в ячейках
		int32 BuildChunkAround = 0;
		int32 SelectRadius = 1;
		bool PathfinderLog = false;
		float PathfinderRotationCost = 1;
		
		TSubclassOf<AHexGridISMActor> HexGridActorClass_ = nullptr;
	};
	static TOptional<FHexGridActorCDODataCache> GetHexGridActorCDODataCache();
	
	void CreateNewChunks(const FVector& InNewPosition);
	
	int32 FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const;

	static uint32 GetColRowCountInChunk();
	
	//InNumColsRows - размеры чанка в ячейках (из настроек)
	void SelectCell(const HexMath::FAxialCoord& InAxialCoord, bool InSelected);
	void SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, bool InSelected);
	void SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel = 1);
	void SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel = 1);
	
	HexMath::FOffsetCoord CurrentChunkCoord_;
	
	UPROPERTY()
	TArray<FChunkData> ChunksList_;
	
	FHashTable ChunkIndexes_;
	
	TArray<HexMath::FAxialCoord> SelectedCells_;
	TArray<HexMath::FAxialCoord> PFCells_;
	
	TSet<HexMath::FAxialCoord> Obstacles_;
	
	TMap<uint32, TArray<HexMath::FAxialCoord>> InfluencedCells_;
};