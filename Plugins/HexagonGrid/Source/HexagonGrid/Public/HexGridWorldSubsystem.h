// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
#include "HexMath.h"
#include "HexPathfainer.h"
#include "MoveParams.h"
#include "Subsystems/WorldSubsystem.h"
#include "HexGridWorldSubsystem.generated.h"


struct FHullHexFootprint;
class UHexagonChunkGrid;

/**
 * 
 */
UCLASS()
class HEXAGONGRID_API UHexGridWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	//static UHexGridWorldSubsystem* Get(const UObject* InWorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UHexagonChunkGrid* GetGrid() { return ChunkGrid_; }

	void OnChangeObserverPosition(const FVector& InNewPosition);

	static HexMath::FAxialCoord WorldToAxialCellCoord(const FVector& InWorldPoint);
	static FVector AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset);
	static FVector AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InHexSize, float InZOffset);
	static float GetHexSize();
	
	static void GetFlatTopHexCorners(const FVector& C, float HexSize, TArray<FVector>& OutCorners);

	static HexMath::FAxialCoord TransformCell(const HexMath::FAxialCoord& InAxialBaseCoord, int8 InRotation, const HexMath::FOffsetCoord& InLocalShift);
	
private:
	
	UPROPERTY()
	UHexagonChunkGrid* ChunkGrid_;	
};
