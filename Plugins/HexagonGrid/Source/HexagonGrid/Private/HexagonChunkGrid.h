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
	
	using FNodeRef = HexMath::FAxialCoord;
	
	bool IsValidRef(FNodeRef NodeRef) const { return NodeRef.Q != INT64_MAX && NodeRef.R != INT64_MAX; }
	FNodeRef GetNeighbour(const FNodeRef& NodeRef, const int32 NeighbourIndex) const;
	
	FVector::FReal GetHeuristicScale() const { return 1; }
	
	FVector::FReal GetHeuristicCost(const FNodeRef& Start, const FNodeRef& End) const
	{
		if (!IsValidRef(Start) || !IsValidRef(End))
		{
			return TNumericLimits<FVector::FReal>::Max();
		}
		return HexMath::AxialDistance(Start, End);
	}
	
	// Стоимость шага (сюда можно подмешать “вес тайла”)
	FVector::FReal GetTraversalCost(const FNodeRef& Start, const FNodeRef& End) const
	{
		return 1;
	}
	
	bool IsTraversalAllowed(const FNodeRef& Start, const FNodeRef& End) const
	{
		return true; //Graph.IsValidRef(B) && !Graph.Blocked.Contains(B);
	}

	bool WantsPartialSolution() const { return false; }

	// Если хочешь, чтобы Start тоже попал в OutPath
	bool ShouldIncludeStartNodeInPath() const { return true; }
	
private:
	static TOptional<HexMath::FOffsetCoord> WorldToChunkCoord(const FVector& InWorldPoint);
	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	
	struct FHexGridActorCDODataCache
	{
		float HexSize = 0;
		int32 NumColsRows = 0;
		int32 BuildChunkAround = 0;
		int32 SelectRadius = 1;
		TSubclassOf<AHexGridISMActor> HexGridActorClass_ = nullptr;
	};
	static TOptional<FHexGridActorCDODataCache> GetHexGridActorCDODataCache();
	
	void CreateNewChunks(const FVector& InNewPosition);
	
	int32 FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const;
	
	void SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows, bool InSelected);
	void SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows, ECellType InCellType);
	
	void FindPath(const HexMath::FAxialCoord& Start, const HexMath::FAxialCoord& End, TArray<HexMath::FAxialCoord>& OutPath);

	HexMath::FOffsetCoord CurrentChunkCoord_;
	
	UPROPERTY()
	TArray<FChunkData> ChunksList_;
	
	FHashTable ChunkIndexes_;
	
	TArray<HexMath::FAxialCoord> SelectedCells_;
};