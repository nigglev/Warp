// Fill out your copyright notice in the Description page of Project Settings.

#include "HexGridWorldSubsystem.h"
#include "HexagonChunkGrid.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"
#include "MoveParams.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridWSLog, Log, Log);

UHexGridWorldSubsystem* UHexGridWorldSubsystem::Get(const UObject* InWorldContextObject)
{
	return InWorldContextObject->GetWorld()->GetSubsystem<UHexGridWorldSubsystem>();
}

void UHexGridWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	ChunkGrid_ = NewObject<UHexagonChunkGrid>(this);
}

void UHexGridWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ChunkGrid_ = nullptr;
}

void UHexGridWorldSubsystem::OnChangeObserverPosition(const FVector& InNewPosition)
{
	ChunkGrid_->OnChangeObserverPosition(InNewPosition);
}

void UHexGridWorldSubsystem::SetCellType(const FVector& InPosition, ECellType InCellType)
{
	ChunkGrid_->SetCellType(InPosition, InCellType);
}

void UHexGridWorldSubsystem::CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull)
{
	ChunkGrid_->CaptureCells(InId, InCenterCell, InRotation, InHull);
}

void UHexGridWorldSubsystem::ReleaseCells(uint32 InId)
{
	ChunkGrid_->ReleaseCells(InId);
}

void UHexGridWorldSubsystem::SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, 
	const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath)
{
	ChunkGrid_->SelectInfluence(InId, InHexCell, InRotation, InMoveParams, OutPath);
}

void UHexGridWorldSubsystem::RemoveInfluence(uint32 InId)
{
	ChunkGrid_->RemoveInfluence(InId);
}

void UHexGridWorldSubsystem::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes) const
{
	ChunkGrid_->FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, OutPath, InDrawHexes);
}

void UHexGridWorldSubsystem::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation,
	const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation, const FMoveParams& InMoveParams,
	float Z, TArray<FVector>& OutPath, bool InDrawHexes) const
{
	static TArray<HexMath::FPathNode> Path;
	Path.Reset();
	
	FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, Path, InDrawHexes);
	
	for (HexMath::FPathNode AC : Path)
	{
		TOptional<FVector> TargetPosOpt = AxialCellToWorldCoord(AC.Coord, Z);
		if (TargetPosOpt.IsSet())
		{
			OutPath.Add(TargetPosOpt.GetValue());
		}
	}
	Path.Reset();
}

void UHexGridWorldSubsystem::DropPathSelections()
{
	ChunkGrid_->DropPathSelections();
}

TOptional<HexMath::FAxialCoord> UHexGridWorldSubsystem::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (Settings->HexGridActorClass_ == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("Settings::HexGridActorClass is not set!"));
		return {};
	}
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (CDO == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("HexGridActor CDO is not valid!"));
		return {};
	}

	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CDO->GetHexSize();
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FAxialCoord CellCoord = HexMathAxial::OffsetToAxial<HEX_LAYOUT>(Coord, HexSize);
	
	return CellCoord;
}

TOptional<FVector> UHexGridWorldSubsystem::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset/* = 0*/)
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (Settings->HexGridActorClass_ == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("Settings::HexGridActorClass is not set!"));
		return {};
	}
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (CDO == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("HexGridActor CDO is not valid!"));
		return {};
	}

	using namespace HexMath;
	using namespace HexMath::HexMathAxial;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CDO->GetHexSize();
	
	FOffsetCoord OffsetCoord = AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	FOffsetRealCoord FC = GetHexOffsetPos<HEX_LAYOUT>(OffsetCoord, HexSize);
	const FVector Loc = HexToWorldSnapped<HEX_LAYOUT>(FC, HexSize, InZOffset);
	
	return Loc;
}
