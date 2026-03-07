// Fill out your copyright notice in the Description page of Project Settings.


#include "HexMapWS.h"

#include "HexGridWorldSubsystem.h"

UHexMapWS* UHexMapWS::Get(const UObject* InWorldContextObject)
{
	return InWorldContextObject->GetWorld()->GetSubsystem<UHexMapWS>();
}

void UHexMapWS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	HexGridWS_ = Collection.InitializeDependency<UHexGridWorldSubsystem>();
}

void UHexMapWS::Deinitialize()
{
	HexGridWS_ = nullptr;
	
	Super::Deinitialize();
}

void UHexMapWS::OnChangeObserverPosition(const FVector& InNewPosition)
{
	if (HexGridWS_)
		HexGridWS_->OnChangeObserverPosition(InNewPosition);
}

void UHexMapWS::SetCellType(const FVector& InPosition, ECellType InCellType)
{
	if (HexGridWS_)
		HexGridWS_->SetCellType(InPosition, InCellType);
}

void UHexMapWS::CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation,
	const FHullHexFootprint& InHull)
{
	if (HexGridWS_)
		HexGridWS_->CaptureCells(InId, InCenterCell, InRotation, InHull);
}

void UHexMapWS::ReleaseCells(uint32 InId)
{
	if (HexGridWS_)
		HexGridWS_->ReleaseCells(InId);
}

void UHexMapWS::SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation,
	const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath)
{
	if (HexGridWS_)
		HexGridWS_->SelectInfluence(InId, InHexCell, InRotation, InMoveParams, OutPath);
}

void UHexMapWS::RemoveInfluence(uint32 InId)
{
	if (HexGridWS_)
		HexGridWS_->RemoveInfluence(InId);
}

void UHexMapWS::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd,
	const TOptional<int8>& InEndRotation, const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath,
	bool InDrawHexes) const
{
	if (HexGridWS_)
		HexGridWS_->FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, OutPath, InDrawHexes);
}

void UHexMapWS::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd,
	const TOptional<int8>& InEndRotation, const FMoveParams& InMoveParams, float Z, TArray<FVector>& OutPath,
	bool InDrawHexes) const
{
	if (HexGridWS_)
		HexGridWS_->FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, Z, OutPath, InDrawHexes);
}

void UHexMapWS::DropPathSelections()
{
	if (HexGridWS_)
		HexGridWS_->DropPathSelections();
}

TOptional<HexMath::FAxialCoord> UHexMapWS::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	return UHexGridWorldSubsystem::WorldToAxialCellCoord(InWorldPoint);
}

TOptional<FVector> UHexMapWS::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset)
{
	return UHexGridWorldSubsystem::AxialCellToWorldCoord(InAxialCoord, InZOffset);
}
