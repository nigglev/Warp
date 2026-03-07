// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacePointer.h"

#include "MGLogs.h"
#include "UnitActors/UnitActorFactory.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/HexMap/HexMapWS.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"

DEFINE_LOG_CATEGORY_STATIC(APlacePointerLog, Log, All);

// Sets default values
APlacePointer::APlacePointer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Root_ = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root_);

	Mesh_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	Mesh_->SetupAttachment(Root_);
	
	ArrowMesh_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh_->SetupAttachment(Root_);
}

void APlacePointer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	RingMat_ = Mesh_->CreateDynamicMaterialInstance(0, Mesh_->GetMaterial(0));
	ArrowMat_ = ArrowMesh_->CreateDynamicMaterialInstance(0, ArrowMesh_->GetMaterial(0));
	
	FixRotation(false);
}

void APlacePointer::Set(TArray<HexMath::FPathNode>&& InPath, ABaseUnitActor* InActiveUnit)
{
	RETURN_ON_FAIL(APlacePointerLog, !InPath.IsEmpty());
	RETURN_ON_FAIL(APlacePointerLog, InActiveUnit != nullptr);
	
	HexMath::FPathNode LastNode = InPath.Last();
	
	if (PathNode_.Coord != LastNode.Coord)
	{
		PathNode_ = LastNode;
		ActiveUnit_ = InActiveUnit;
		
		AxialAngle_.R = LastNode.Rotation;
	
		TOptional<FVector> PosOpt = UHexMapWS::AxialCellToWorldCoord(PathNode_.Coord);
		RETURN_ON_FAIL(APlacePointerLog, PosOpt.IsSet());
	
		const FRotator WorldRot(0.f, AxialAngle_.GetYaw(), 0.f);
	
		SetActorLocationAndRotation(PosOpt.GetValue(), WorldRot);
		
		FixRotation(false);
		RingMat_->SetVectorParameterValue(TEXT("BaseColor"), StartColor_);
		
		OnTransformChanged();
		
		if (Ghost_ != nullptr)
			Ghost_->SetCirclePath(MoveTemp(InPath));
	}
	else
		FixRotation(!bRotationFixed_);
}

void APlacePointer::FixRotation(bool InFixed)
{
	bRotationFixed_ = InFixed;
	RingMat_->SetVectorParameterValue(TEXT("BaseColor"), bRotationFixed_ ? FixedColor_ : StartColor_);
}

void APlacePointer::FixRotation()
{
	FixRotation(true);
}

void APlacePointer::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
	
	RETURN_ON_FAIL(APlacePointerLog, !GetWorld()->IsNetMode(NM_DedicatedServer));
	
	if (!bRotationFixed_)
	{
		TryChangeAngle();
	
		UpdateRotation(InDeltaTime);
	}
}

void APlacePointer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Ghost_ != nullptr)
		Ghost_->Destroy();
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	RETURN_ON_FAIL(APlacePointerLog, HexMapWS != nullptr);
	HexMapWS->ReleaseCells(GetUniqueID());
	
	Super::EndPlay(EndPlayReason);
}

void APlacePointer::TryChangeAngle()
{
	RETURN_ON_FAIL(APlacePointerLog, IsValid(ActiveUnit_));
	RETURN_ON_FAIL(APlacePointerLog, ActiveUnit_->GetDescription() != nullptr);
	
	auto PC = Cast<ADefaultPlayerController>(GetWorld()->GetFirstPlayerController());
	
	FVector OwnLocation = GetActorLocation();
	FVector TargetLocation;
	if (PC->GetMouseRayPlaneZIntersection(OwnLocation.Z, TargetLocation))
	{
		FVector Dir;
		float Dist;
	
		(TargetLocation - OwnLocation).ToDirectionAndLength(Dir, Dist);
	
		if (Dist > DeadZone_)
		{
			const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
			
			int8 DirAngle = FAxialAngle::GetDirectionAngle(Yaw);
			
			float RotationDist = HexMath::GetRotationDiff(PathNode_.Rotation, DirAngle) * ActiveUnit_->GetDescription()->MoveParams.RotationCost;
			
			float RestDist = ActiveUnit_->GetDescription()->MoveParams.MaxDistance - PathNode_.Distance;
			
			if (RestDist >= RotationDist)
			{
				AxialAngle_.SetByYaw(Yaw);
			}
		}
	}
}

void APlacePointer::UpdateRotation(float InDelta)
{
	float CurrentYaw = CurrentYaw = FMath::UnwindDegrees(GetActorRotation().Yaw);
	float TargetYaw  = FMath::UnwindDegrees(AxialAngle_.GetYaw());
	
	if (!FMath::IsNearlyEqual(CurrentYaw, AxialAngle_.GetYaw()))
	{
		const float NewYaw = FMath::FixedTurn(CurrentYaw, TargetYaw, RotateSpeed_ * InDelta);
		const FRotator WorldRot(0.f, NewYaw, 0.f);
		SetActorRotation(WorldRot);
	
		OnTransformChanged();
	}
}

void APlacePointer::OnTransformChanged()
{
	if (CreateGhost_)
	{
		if (Ghost_ == nullptr)
		{
			Ghost_ = UnitActorFactory::CreateUnitActor(this, ActiveUnit_->GetUnitType(), 
			   FAxialTransform(PathNode_.Coord, AxialAngle_), this, true);
		}
		else
			Ghost_->SetLastRotation(AxialAngle_);
	}
	
	const FUnitDescription* Descr = ActiveUnit_->GetDescription();
	RETURN_ON_FAIL(APlacePointerLog, Descr != nullptr);
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	RETURN_ON_FAIL(APlacePointerLog, HexMapWS != nullptr);
	
	HexMapWS->CaptureCells(GetUniqueID(), PathNode_.Coord, AxialAngle_.R, Descr->Footprint);
}