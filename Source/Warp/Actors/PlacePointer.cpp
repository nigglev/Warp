// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacePointer.h"

#include "MGLogs.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"

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

// Called when the game starts or when spawned
void APlacePointer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlacePointer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RETURN_ON_FAIL(APlacePointerLog, !GetWorld()->IsNetMode(NM_DedicatedServer));
	
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
			AxialAngle_.SetByYaw(Yaw);
		}
	}
	
	float CurrentYaw = CurrentYaw = FMath::UnwindDegrees(GetActorRotation().Yaw);
	float TargetYaw  = FMath::UnwindDegrees(AxialAngle_.GetYaw());
	
	if (!FMath::IsNearlyEqual(CurrentYaw, AxialAngle_.GetYaw()))
	{
		const float NewYaw = FMath::FixedTurn(CurrentYaw, TargetYaw, RotateSpeed_ * DeltaTime);
		const FRotator WorldRot(0.f, NewYaw, 0.f);
		SetActorRotation(WorldRot);
	}
}

