// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "HexGridWorldSubsystem.h"
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
	DOREPLIFETIME(ABaseUnitActor, AxialCoord_);
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseUnitActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || Path_.IsEmpty())
	{
		return;
	}

	FVector Current = GetActorLocation();
	
	FVector Target = Path_[0];
	
	FVector Dir = Target - Current;
	float CurrentDist;
	Dir.ToDirectionAndLength(Dir, CurrentDist);
	
	float ShiftLen = MoveSpeed_ * DeltaSeconds;
	if (CurrentDist <= ShiftLen)
	{
		ShiftLen = CurrentDist;
		Path_.RemoveAt(0);
	}

	FVector NewLoc = Current + Dir * ShiftLen;

	SetActorLocation(NewLoc, true);
}


void ABaseUnitActor::SetMoveTarget(const FRepAxialCoord& InTarget)
{
	if (!HasAuthority())
	{
		return;
	}
	
	MG_COND_ERROR_SHORT(ABaseUnitActorLog, !Path_.IsEmpty());
	Path_.Reset();
	
	UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
	
	TArray<HexMath::FAxialCoord> Path;
	GridWorldSubsystem->SelectedFindPath(AxialCoord_.ToNative(), InTarget.ToNative(), Path);
	
	FVector Current = GetActorLocation();
	for (HexMath::FAxialCoord AC : Path)
	{
		TOptional<FVector> TargetPosOpt = UHexGridWorldSubsystem::AxialCellToWorldCoord(AC, Current.Z);
		if (TargetPosOpt.IsSet())
		{
			Path_.Add(TargetPosOpt.GetValue());
		}
	}
	
	if (!Path_.IsEmpty())
	{
		AxialCoord_ = InTarget;
	}
}


void ABaseUnitActor::OnRep_UnitActorSize()
{

}


void ABaseUnitActor::OnRep_UnitType()
{
	
}