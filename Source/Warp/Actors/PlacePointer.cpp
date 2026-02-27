// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacePointer.h"

#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
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

void APlacePointer::Set(const HexMath::FPathNode& InPathNode, ABaseUnitActor* InActiveUnit)
{
	RETURN_ON_FAIL(APlacePointerLog, InActiveUnit != nullptr);
	
	if (PathNode_.Coord != InPathNode.Coord)
	{
		PathNode_ = InPathNode;
		ActiveUnit_ = InActiveUnit;
	
		AxialAngle_.R = InPathNode.Rotation;
	
		TOptional<FVector> PosOpt = UHexGridWorldSubsystem::AxialCellToWorldCoord(PathNode_.Coord);
		RETURN_ON_FAIL(APlacePointerLog, PosOpt.IsSet());
	
		const FRotator WorldRot(0.f, AxialAngle_.GetYaw(), 0.f);
	
		SetActorLocationAndRotation(PosOpt.GetValue(), WorldRot);
		
		FixRotation(false);
		RingMat_->SetVectorParameterValue(TEXT("BaseColor"), StartColor_);
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
			
			float RotationDist = HexMath::GetRotationDiff(PathNode_.Rotation, DirAngle) * ActiveUnit_->GetDescription()->RotationCost;
			
			float RestDist = ActiveUnit_->GetDescription()->MaxRoundDistance - PathNode_.Distance;
			
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
	}
}
