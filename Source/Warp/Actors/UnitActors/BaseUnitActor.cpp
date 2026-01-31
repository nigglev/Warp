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

	Root_ = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root_);

	Mesh_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh_->SetupAttachment(Root_);
}

void ABaseUnitActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseUnitActor, UnitType_);
	DOREPLIFETIME(ABaseUnitActor, UnitActorSize_);
	DOREPLIFETIME(ABaseUnitActor, MoveTarget_);
	DOREPLIFETIME(ABaseUnitActor, bHasMoveTarget_);
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseUnitActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bHasMoveTarget_)
	{
		return;
	}

	FVector Current = GetActorLocation();
	FVector ToTarget = FVector(MoveTarget_.X - Current.X, MoveTarget_.Y - Current.Y, 0.f);

	const float DistSq = ToTarget.SizeSquared();
	if (DistSq <= FMath::Square(AcceptanceRadius_))
	{
		bHasMoveTarget_ = false;
		ForceNetUpdate();
		return;
	}

	const FVector Dir = ToTarget.GetSafeNormal();
	FVector NewLoc = Current + Dir * MoveSpeed_ * DeltaSeconds;
	NewLoc.Z = Current.Z;

	SetActorLocation(NewLoc, true);
}


void ABaseUnitActor::SetMoveTarget(const FVector& InTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	MoveTarget_ = FVector(InTarget.X, InTarget.Y, GetActorLocation().Z);
	bHasMoveTarget_ = true;

	ForceNetUpdate();
}


void ABaseUnitActor::OnRep_UnitActorSize()
{

}


void ABaseUnitActor::OnRep_UnitType()
{
	
}