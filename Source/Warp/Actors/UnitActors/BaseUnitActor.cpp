// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(ABaseUnitActorLog, Log, All);

// Sets default values
ABaseUnitActor::ABaseUnitActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
	if (MeshComponent)
	{
		DefaultMaterial = MeshComponent->GetMaterial(0);
	}
}

void ABaseUnitActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseUnitActor, UnitID);
}

void ABaseUnitActor::ResizeMeshToSize(uint32 TargetLength, uint32 TargetWidth) const
{
	const FBox LocalBox = MeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector RawSize = LocalBox.GetSize();	

	const float ScaleX = TargetLength / RawSize.X;
	const float ScaleY = TargetWidth  / RawSize.Y;
	const float ScaleZ = MeshComponent->GetRelativeScale3D().Z;

	MeshComponent->SetRelativeScale3D(FVector(ScaleX, ScaleY, ScaleZ));
}

void ABaseUnitActor::ResizeMeshToSize(const FVector& InScale) const
{
	MeshComponent->SetRelativeScale3D(InScale);
}

void ABaseUnitActor::UpdatePosition(const FVector& InPosition)
{
	MG_COND_LOG(ABaseUnitActorLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("Placing Unit at {%s}"),  *InPosition.ToString());
	SetActorLocation(InPosition + FVector(0, 0, 50.0f));
}

void ABaseUnitActor::Rotate(bool InClockwise)
{
	if (InClockwise)
	{	
		UnitActorRotation.RotateClockwise();
		SetActorRotation(FRotator(0, UnitActorRotation.GetUnitFRotation(), 0));
	}
	if (!InClockwise)
	{
		UnitActorRotation.RotateCounterClockwise();
		SetActorRotation(FRotator(0, UnitActorRotation.GetUnitFRotation(), 0));
	}
}

void ABaseUnitActor::SetHighlighted(const bool bOn) const
{
	if (!MeshComponent) 
		return;

	if (bOn && HighlightMaterial)
	{
		MeshComponent->SetMaterial(0, HighlightMaterial);
	}
	else if (!bOn && DefaultMaterial)
	{
		MeshComponent->SetMaterial(0, DefaultMaterial);
	}
}


