// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
#include "HexMath.h"
#include "HexPathfainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "HexGridWorldSubsystem.generated.h"


class UHexagonChunkGrid;

/**
 * 
 */
UCLASS()
class HEXAGONGRID_API UHexGridWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	static UHexGridWorldSubsystem* Get(const UObject* InWorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnChangeObserverPosition(const FVector& InNewPosition);
	void SelectCell(const FVector& InPosition);
	void SetCellType(const FVector& InPosition, ECellType InCellType);
	
	void SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, float InHexDistance, float InMoveCost, float InRotationCost, 
		TArray<HexMath::FPathNode>* OutPath = nullptr);
	void RemoveInfluence(uint32 InId);
	
	void FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		float InMaxDistance, TArray<HexMath::FPathNode>& OutPath, float InMoveCost, float InRotationCost, bool InDrawHexes) const;
	
	void DropPathSelections();

	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	static TOptional<FVector> AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset = 0);
	
private:
	
	UPROPERTY()
	UHexagonChunkGrid* ChunkGrid_;	
};
