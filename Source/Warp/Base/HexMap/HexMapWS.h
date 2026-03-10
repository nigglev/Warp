// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECellType.h"
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
	
	bool CaptureCells(uint32 InId, uint32 InUnitId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull, bool InOnlyCheck);
	void ReleaseCells(uint32 InId);
	
	void SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath = nullptr);
	void RemoveInfluence(uint32 InId);
	
	void FindPath(uint32 InId, const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes);
	
	void DropPathSelections(uint32 InId);
	
	bool IsDenyToCapture(uint32 InId, const HexMath::FAxialCoord& InCoord) const;
	
	static TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	static TOptional<FVector> AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset = 0);
	
private:
	struct FCollectionKey
	{
		uint32 Id = TNumericLimits<uint32>::Max();
		ECellType CellType = ECellType::Opened;
		
		FORCEINLINE bool operator == (const FCollectionKey& Rhs) const
		{
			return Id == Rhs.Id && CellType == Rhs.CellType;
		}

		FORCEINLINE bool operator != (const FCollectionKey& Rhs) const
		{
			return !(*this == Rhs);
		}

		FORCEINLINE bool operator < (const FCollectionKey& Rhs) const
		{
			if (Id < Rhs.Id)
				return true;
			if (Id > Rhs.Id)
				return false;
			return CellType < Rhs.CellType;
		}
	};
	
	struct FIdCellCollection
	{
		FCollectionKey Key;
		TArray<HexMath::FAxialCoord> Cells;
		FIdCellCollection() = default;
		FIdCellCollection(FCollectionKey InKey) : Key(InKey) {}
	};
	
	struct FCellIdData
	{
		uint32 Id = TNumericLimits<uint32>::Max();
		float Level = 0;
	};

	struct FCell
	{
		TArray<FCellIdData> Counters[static_cast<int32>(ECellType::MAX)];
	};
	
	FIdCellCollection* GetIdCellCollection(FCollectionKey InKey, bool InCreate);
	
	void ChangeCell(const HexMath::FAxialCoord& InCellCoord, ECellType InCellType, FCellIdData InData);
	bool TryClearCell(FCell& InCell);
	void OnChangeCell(const HexMath::FAxialCoord& InCellCoord, FCell& InCell);
	
	void ClearCells(uint32 InId, ECellType InCellType);
	
	TArray<FIdCellCollection> IdCellCollections_;
	
	TMap<HexMath::FAxialCoord, FCell> Cells_;
	
	UPROPERTY()
	UHexGridWorldSubsystem* HexGridWS_;
};
