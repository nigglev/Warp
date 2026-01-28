// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Misc/MapErrors.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(ABaseUnitActorLog, Log, All);

ABaseUnitActor::ABaseUnitActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
}

void ABaseUnitActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseUnitActor, UnitActorSize);
	DOREPLIFETIME(ABaseUnitActor, MoveTarget);
	DOREPLIFETIME(ABaseUnitActor, bHasMoveTarget);
}


void ABaseUnitActor::PostNetInit()
{
	Super::PostNetInit();
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseUnitActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bHasMoveTarget)
	{
		return;
	}

	FVector Current = GetActorLocation();
	FVector ToTarget = FVector(MoveTarget.X - Current.X, MoveTarget.Y - Current.Y, 0.f);

	const float DistSq = ToTarget.SizeSquared();
	if (DistSq <= FMath::Square(AcceptanceRadius))
	{
		bHasMoveTarget = false;
		ForceNetUpdate();
		return;
	}

	const FVector Dir = ToTarget.GetSafeNormal();
	FVector NewLoc = Current + Dir * MoveSpeed * DeltaSeconds;
	NewLoc.Z = Current.Z;

	SetActorLocation(NewLoc, true);
}


void ABaseUnitActor::SetMoveTarget(const FVector& InTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	MoveTarget = FVector(InTarget.X, InTarget.Y, GetActorLocation().Z);
	bHasMoveTarget = true;

	ForceNetUpdate();
}



void ABaseUnitActor::OnRep_UnitActorSize()
{

}
