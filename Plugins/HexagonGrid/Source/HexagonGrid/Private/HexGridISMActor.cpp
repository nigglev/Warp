#include "HexGridISMActor.h"
#include "HexMath.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridActorLog, Log, Log);

using namespace HexMathOffset;

#define HEX_LAYOUT HexMath::EHexOffsetLayout::FlatTopOddQ

AHexGridISMActor::AHexGridISMActor()
{
	PrimaryActorTick.bCanEverTick = true;

	auto* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ISM_ = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
	ISM_->SetupAttachment(Root);

	ISM_->SetMobility(EComponentMobility::Type::Movable);

	// Для “сетки везде” лучше без коллизии:
	ISM_->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ISM_->SetCanEverAffectNavigation(false);
	
	ISM_->SetUsingAbsoluteLocation(false);
	ISM_->SetUsingAbsoluteRotation(false);
	ISM_->SetUsingAbsoluteScale(false);

	ISM_->NumCustomDataFloats = 5; // R,G,B,A, ZOffset
}

void AHexGridISMActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	Rebuild();
}

void AHexGridISMActor::BeginPlay()
{
	Super::BeginPlay();

	Rebuild();
}

void AHexGridISMActor::Rebuild()
{
	ISM_->ClearInstances();

	if (!HexTileMesh_) 
		return;
	
	ISM_->SetStaticMesh(HexTileMesh_);
	
	if (Material_)
	{
		ISM_->SetMaterial(0, Material_);
	}
	
	BuildHexagon(GridSize_);
}

void AHexGridISMActor::BuildHexagon(uint32 InHexWidth)
{
	// число тайлов: 1 + 3R(R+1)
	const uint32 Expected = 1 + 3 * InHexWidth * (InHexWidth + 1);
	
	TArray<FTransform> Transforms;
	Transforms.Reserve(Expected);
	
	const FRotator R90(0, 90, 0);
	FRotator R = HexRotation_ ? R90 : FRotator::ZeroRotator;

	HexMath::HexMathOffset::BuildHexagonGrid<HEX_LAYOUT>(InHexWidth, HexSize_, 
		[&Transforms, &R, DetailDebug = DetailDebug_, SizeScale = SizeScale_]
		(const HexMath::FOffsetCoord& InOCoord, const FVector& InLoc)
	{
		if (DetailDebug)
		{
			UE_LOG(HexGridActorLog, Warning, TEXT("%llu : %llu\t%5.2f : %2.2f"), InOCoord.Col, InOCoord.Row, InLoc.X, InLoc.Y);
		}
		 
		const FTransform Tr(R, InLoc, FVector(SizeScale, SizeScale, 1));
		Transforms.Emplace(Tr);
	});
	
	const int32 BaseIndex = ISM_->GetInstanceCount();
	
	// bulk add
	ISM_->AddInstances(Transforms, /*bShouldReturnIndices=*/false, /*bWorldSpace=*/false);
	
	for (int32 i = 0; i < Transforms.Num(); ++i)
	{
		const int32 Idx = BaseIndex + i;
		FLinearColor Clr = GetColor(Idx);
		float ZOffset = GetZOffset(Idx);
		SetHexColor(Idx, Clr);
		SetHexZOffset(Idx, ZOffset);
	}
	
	UpdateMPC();

	ISM_->MarkRenderStateDirty();

	if (DebugBoundsTime_ > 0)
	{
		FVector Pos = GetActorLocation() + GetExtent();
		DrawDebugBox(GetWorld(), Pos, GetExtent(), FColor::Green, false, DebugBoundsTime_);
	}
}

void AHexGridISMActor::SetHexColor(int32 InIndex, const FLinearColor& InColor) const
{
	if (ISM_ == nullptr)
		return;
	if (!ensure(InIndex >= 0))
		return;
	if (!ensure(InIndex < ISM_->GetNumInstances()))
		return;
	
	ISM_->SetCustomDataValue(InIndex, 0, InColor.R, false);
	ISM_->SetCustomDataValue(InIndex, 1, InColor.G, false);
	ISM_->SetCustomDataValue(InIndex, 2, InColor.B, false);
	ISM_->SetCustomDataValue(InIndex, 3, InColor.A, false);
}

void AHexGridISMActor::SetHexZOffset(int32 InIndex, float InZOffset) const
{
	if (ISM_ == nullptr)
		return;
	if (!ensure(InIndex >= 0))
		return;
	if (!ensure(InIndex < ISM_->GetNumInstances()))
		return;
	
	ISM_->SetCustomDataValue(InIndex, 4, InZOffset, false);
}

void AHexGridISMActor::UpdateMPC()
{
	if (!ensure(MPC_))
		return;

	UWorld* World = GetWorld();
	if (!ensure(World))
		return;

	UMaterialParameterCollectionInstance* Inst = World->GetParameterCollectionInstance(MPC_);
	if (!ensure(Inst))
		return;
	
	Inst->SetScalarParameterValue(TEXT("FadeStartRadius"), FadeStartRadius_);
	Inst->SetScalarParameterValue(TEXT("FadeLength"), FadeLength_);
}

FVector AHexGridISMActor::GetExtent() const
{
	HexMath::FOffsetRealCoord SZ = HexMath::HexMathOffset::GetChunkPitchNoGap<HEX_LAYOUT>(GridSize_, GridSize_, HexSize_);
	FVector fSZ = HexMath::HexMathOffset::OffsetHexToWorld(SZ);
	return fSZ / 2;
}

void AHexGridISMActor::SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel)
{
	HexMath::FOffsetCoord LocalCoord = InOffsetCoord - ChunkCoord_ * GridSize_;
	
	if (!ensure(LocalCoord.Col >= 0 && LocalCoord.Row >= 0))
		return;
	
	UE_LOG(HexGridActorLog, Verbose, TEXT("Chunk: %s; LocalORCoord: %s; LocalOCoord: %s"), 
		*ChunkCoord_.ToString(), *LocalCoord.ToString(), *LocalCoord.ToString());
	
	int32 Index = GridSize_ * LocalCoord.Row + LocalCoord.Col;
	SetCellType(Index, InCellType, InLevel);
}

void AHexGridISMActor::SetCellType(int32 InIndex, ECellType InCellType, float InLevel)
{
	ChangeCellStatus(InIndex, InCellType, InLevel);
	
	FLinearColor Clr = GetColor(InIndex);
	float ZOffset = GetZOffset(InIndex);
		
	SetHexColor(InIndex, Clr);
	SetHexZOffset(InIndex, ZOffset);
}

void AHexGridISMActor::ChangeCellStatus(int32 InIndex, ECellType InCellType, float InLevel)
{
	FCellLayers* Cache = SelectStatus_.Find(InIndex);
	if (Cache == nullptr && InLevel == 0)
		return;
	
	if (Cache == nullptr)
	{
		Cache = &SelectStatus_.Emplace(InIndex);
	}
	
	Cache->SetCellType(InCellType, InLevel);
	if (Cache->GetCellType() == ECellType::Opened)
	{
		SelectStatus_.Remove(InIndex);
	}
}

FLinearColor AHexGridISMActor::GetColor(int32 InIndex) const
{
	const FCellLayers* Status = SelectStatus_.Find(InIndex);
	
	const FLinearColor* OpenedColor = CellColors_.Find(ECellType::Opened);
	if (!ensure(OpenedColor))
		return FLinearColor::White;
	
	if (Status == nullptr)
	{
		return *OpenedColor;
	}
	
	const FLinearColor* StatusColor = CellColors_.Find(Status->GetCellType());
	if (!ensure(StatusColor))
		return FLinearColor::White;
	
	return FMath::Lerp(*OpenedColor, *StatusColor, Status->GetMaxLevel());
	//return FLinearColor::LerpUsingHSV(OpenedColor, StatusColor, Status->Level);
}

float AHexGridISMActor::GetZOffset(int32 InIndex) const
{
	const FCellLayers* Status = SelectStatus_.Find(InIndex);
	int32 SellTypeIndex = Status ? Status->MaxIndex() : 0;
	return ZOffset_ * SellTypeIndex;
}

void AHexGridISMActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!ensure(MPC_))
		return;

	UWorld* World = GetWorld();
	if (!ensure(World))
		return;

	UMaterialParameterCollectionInstance* Inst = World->GetParameterCollectionInstance(MPC_);
	if (!ensure(Inst))
		return;
	
	FVector CameraPos = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	
	Inst->SetVectorParameterValue(TEXT("ObserverWorldPos"), CameraPos);
}