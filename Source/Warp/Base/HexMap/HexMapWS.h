// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HexMath.h"
#include "HexPathfainer.h"
#include "HexMapWS.generated.h"

enum class ECellType : uint8;
class UHexGridWorldSubsystem;
/**
 * 
 */
UCLASS()
class WARP_API UHexMapWS : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static UHexMapWS* Get(const UObject* InWorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnChangeObserverPosition(const FVector& InNewPosition);
	void SetCellType(const FVector& InPosition, ECellType InCellType);
	
	void CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull);
	void ReleaseCells(uint32 InId);
	
	void SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath = nullptr);
	void RemoveInfluence(uint32 InId);
	
	void FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes) const;
	
	void FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, float Z, TArray<FVector>& OutPath, bool InDrawHexes) const;
	
	void DropPathSelections();
	
	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	static TOptional<FVector> AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset = 0);
	
private:
	UPROPERTY()
	UHexGridWorldSubsystem* HexGridWS_;
};
