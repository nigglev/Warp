// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatMap.h"

#include "GroomVisualizationData.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Units/UnitBase.h"

DEFINE_LOG_CATEGORY_STATIC(UCombatMapLog, Log, All);

void UCombatMap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UCombatMap, GridSize, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UCombatMap, TileSize, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UCombatMap, OccupiedTiles, RepParams);
}

void UCombatMap::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		InitializeFreeTiles();
	}
}

void UCombatMap::InitializeFreeTiles()
{
	FreeTiles.Empty();
	const uint32 Total = GridSize * GridSize;
	FreeTiles.Reserve(Total);
	
	for (uint32 X = 0; X < GridSize; ++X)
	{
		for (uint32 Y = 0; Y < GridSize; ++Y)
		{
			FCombatMapTile T = MakeTile(X, Y);
			FreeTiles.Add(T);
		}
	}

	OccupiedTiles.Empty();
}

void UCombatMap::PlaceUnitAt(UUnitBase* InUnitToPlace, const FIntVector2& InCenter)
{
	if (!InUnitToPlace)
	{
		MG_COND_ERROR(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("PlaceUnitOnMapRand: null unit"));
		return;
	}
	
	FCombatMapTile CenterTile = MakeTile(InCenter.X, InCenter.Y);
	
	TArray<FCombatMapTile> Footprint;
	if (ExtractFootprint(CenterTile, InUnitToPlace->UnitSize, InUnitToPlace->UnitRotation, Footprint))
		SetupUnitPosOnMap(InUnitToPlace, CenterTile, Footprint);
}


void UCombatMap::PlaceUnitOnMapRand(UUnitBase* InUnitToPlace)
{
	if (!InUnitToPlace)
	{
		MG_COND_ERROR(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("PlaceUnitOnMapRand: null unit"));
		return;
	}
	
	TArray<FCombatMapTile> OutFootprint;
	const FCombatMapTile Center = GetRandomPlaceableTile(InUnitToPlace->UnitSize, InUnitToPlace->UnitRotation, OutFootprint);
	SetupUnitPosOnMap(InUnitToPlace, Center, OutFootprint);
}

bool UCombatMap::CheckPositionForUnit(const FIntVector2& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const
{
	TArray<FCombatMapTile> OutFootprint;
	return ExtractFootprint(MakeTile(InUnitCenter.X, InUnitCenter.Y), InUnitSize, InUnitRotation, OutFootprint, &OutBlockers);
}


void UCombatMap::SetupUnitPosOnMap(UUnitBase* InUnitToPlace, const FCombatMapTile& InUnitCenter, const TArray<FCombatMapTile>& InFootprint)
{
	InUnitToPlace->UnitPosition.SetUnitPosition(InUnitCenter.WorldPosition, FIntVector2(InUnitCenter.X, InUnitCenter.Y));
	ModifyTilesOccupancy(InFootprint, InUnitToPlace,true);
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("Placed unit [%s] at a tile = [%d, %d] [%f, %f]"), *InUnitToPlace->ToString(), InUnitCenter.X, InUnitCenter.Y, InUnitCenter.WorldPosition.X, InUnitCenter.WorldPosition.Y);
	LogFootprint(InFootprint, false);
}


void UCombatMap::RemoveUnitFromMap(UUnitBase* InUnitToRemove)
{
	if (!InUnitToRemove)
	{
		MG_COND_ERROR(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("PlaceUnitOnMapRand: null unit"));
	}

}

FCombatMapTile UCombatMap::GetRandomPlaceableTile(const FUnitSize& InUnitSize,
	const FUnitRotation& InUnitRotation, TArray<FCombatMapTile>& OutFootprint) const
{
	TArray<FCombatMapTile> OutBlockers;
	int Attempts = GridSize * GridSize;
	while (Attempts-- > 0)
	{
		
		int x = FMath::RandRange(0, static_cast<int>(GridSize)-1);
		int y = FMath::RandRange(0, static_cast<int>(GridSize)-1);
		MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("Trying to place at [%d, %d]"), x, y);
		FCombatMapTile Cand = MakeTile(x,y);
		if (ExtractFootprint(Cand, InUnitSize, InUnitRotation, OutFootprint))
		{
			MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("[%d, %d] is valid"), x, y);
			return Cand;
		}
			
	}
	return FCombatMapTile();
}

bool UCombatMap::ExtractFootprint(const FCombatMapTile& InCenterTile, const FUnitSize& InUnitSize,
                                  const FUnitRotation& InUnitRotation, TArray<FCombatMapTile>& OutFootprint,
                                  TArray<FIntPoint>* OutBlockers) const
{
	OutFootprint.Empty();
	bool IsOutOfBounds = false;
	bool IsOnOccupiedTiles = false;
	int32 Half = InUnitSize.GetUnitTileLength().X / 2;
	FIntVector2 Delta = GetMovementDelta(InUnitRotation.GetUnitRotation());

	for (int32 Step = -Half; Step <= Half; ++Step)
	{
		int32 X = InCenterTile.X + Step * Delta.X;
		int32 Y = InCenterTile.Y + Step * Delta.Y;
		FCombatMapTile NewTile = MakeTile(X,Y);
		if (X < 0 || Y < 0 || X >= static_cast<int32>(GridSize) || Y >= static_cast<int32>(GridSize))
		{
			MG_COND_ERROR(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("Unit is out of bounds"));
			IsOutOfBounds = true;
			continue;
		}
		if (OccupiedTiles.Contains(NewTile))
		{
			MG_COND_ERROR(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("Tile [%d, %d] already occupied"),  NewTile.X, NewTile.Y);
			if (OutBlockers != nullptr)
			{
				IsOnOccupiedTiles = true;
				OutBlockers->Add(FIntPoint(X, Y));	
			}
			continue;
		}
		OutFootprint.Add(NewTile);
	}
	return (IsOutOfBounds == false) && (IsOnOccupiedTiles == false);
}


FIntVector2 UCombatMap::GetMovementDelta(const EUnitRotation InUnitRotation)
{
	FIntVector2 Delta;
	switch (InUnitRotation)
	{
	case EUnitRotation::Rot_0:		Delta = { +1,  0 }; return Delta;
	case EUnitRotation::Rot_45:		Delta = { +1, +1 }; return Delta;
	case EUnitRotation::Rot_90:		Delta = {  0, +1 }; return Delta;
	case EUnitRotation::Rot_135:	Delta = { -1, +1 }; return Delta;
	case EUnitRotation::Rot_180:	Delta = { -1,  0 }; return Delta;
	case EUnitRotation::Rot_225:	Delta = { -1, -1 }; return Delta;
	case EUnitRotation::Rot_270:	Delta = {  0, -1 }; return Delta;
	case EUnitRotation::Rot_315:	Delta = { +1, -1 }; return Delta;
	case EUnitRotation::Max:		Delta = { 0, 0 }; return Delta;
	default:						Delta = { +1,  0 }; return Delta;
	}
}


FCombatMapTile UCombatMap::MakeTile(int32 X, int32 Y) const
{
	FCombatMapTile Tile;
	Tile.X = X;
	Tile.Y = Y;
	Tile.WorldPosition = FVector2f(
		X * TileSize + TileSize * 0.5f,
		Y * TileSize + TileSize * 0.5f
	);
	return Tile;
}

FCombatMapTile UCombatMap::MakeTileWithUnitID(int32 X, int32 Y, uint8 InUnitID) const
{
	FCombatMapTile Tile = MakeTile(X, Y);
	Tile.PlacedUnitID = InUnitID;
	return Tile;
}

void UCombatMap::ModifyTilesOccupancy(const TArray<FCombatMapTile>& Tiles, const UUnitBase* Unit, const bool bAdd)
{
	for (auto& T : Tiles)
	{
		if (bAdd)
		{
			FCombatMapTile NewTile = MakeTileWithUnitID(T.X, T.Y, Unit->GetUnitCombatID());
			OccupiedTiles.Add(NewTile);
			MARK_PROPERTY_DIRTY_FROM_NAME(UCombatMap, OccupiedTiles, this);
			MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
			TEXT("Added Tile [%d, %d] with unit id = %d; Current number of occupied tiles: %d"), NewTile.X, NewTile.Y, NewTile.PlacedUnitID, OccupiedTiles.Num());
			FreeTiles.RemoveSingleSwap(NewTile);
		}
		else
		{
			OccupiedTiles.Remove(T);
			FreeTiles.Add(T);
		}
	}
}

void UCombatMap::LogFootprint(const TArray<FCombatMapTile>& InFootprint, bool IsFreed)
{
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap), TEXT("START"));
	if (IsFreed)
	{
		for (int i = 0; i < InFootprint.Num(); ++i)
		{
			MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
				TEXT("Tile = [%d, %d] are now free"), InFootprint[i].X, InFootprint[i].Y);
		}	
	}
	else
	{
		for (int i = 0; i < InFootprint.Num(); ++i)
		{
			MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
				TEXT("Tile = [%d, %d] are now occupied"), InFootprint[i].X, InFootprint[i].Y);
		}	
	}
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap), TEXT("END"));
	
}

void UCombatMap::OnRep_GridSize()
{
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
		TEXT("Name = %s: Replicated Grid Size. Size = %d"), *GetName(), GridSize);
}

void UCombatMap::OnRep_TileSize()
{
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
		TEXT("Name = %s: Replicated Tile Size. Size = %d"), *GetName(), TileSize);
}

void UCombatMap::OnRep_OccupiedTiles()
{
	MG_COND_LOG(UCombatMapLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap),
		TEXT("Name = %s: Replicated occupied tiles. Size = %d"), *GetName(), OccupiedTiles.Num());
}
