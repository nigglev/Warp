// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "HexGridWorldSubsystem.h"
#include "HexPathfainer.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Misc/MapErrors.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"

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
	DOREPLIFETIME(ABaseUnitActor, bOnMove_);
	DOREPLIFETIME(ABaseUnitActor, AxialCoord_);
	DOREPLIFETIME(ABaseUnitActor, AxialAngle_);
}

bool ABaseUnitActor::IsLoaded() const
{
	return UnitType_ != NAME_None;
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseUnitActor::Tick(float InDelta)
{
	Super::Tick(InDelta);

	if (!HasAuthority() || !bOnMove_)
	{
		return;
	}
	
	if (Path_.IsValidIndex(PathIndex_))
	{
		FVector Target = Path_[PathIndex_];
		EMoveState MoveState = MoveToTarget(InDelta, Target);
		if (MoveState == EMoveState::Approached)
		{
			PathIndex_++;
		}
	}
	else
	{
		float TargetYaw  = FMath::UnwindDegrees(AxialAngle_.GetYaw());
		bOnMove_ = UpdateRotation(InDelta, TargetYaw);
		
		if (!bOnMove_)
		{
			auto GS = Cast<AWarpGameState>(GetWorld()->GetGameState());
			RETURN_ON_FAIL(ABaseUnitActorLog, GS);
			GS->OnUnitArrived.Broadcast(this);
		}		
	}
}

ABaseUnitActor::EMoveState ABaseUnitActor::MoveToTarget(float InDelta, const FVector& Target)
{
	FVector Current = GetActorLocation();
	
	FVector Dir = Target - Current;
	float CurrentDist;
	Dir.ToDirectionAndLength(Dir, CurrentDist);
	
	bool Approached = FMath::IsNearlyZero(CurrentDist, 1.e-3f);
	if (Approached)
		return EMoveState::Approached;
	
	float TargetYaw  = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));

	if (UpdateRotation(InDelta, TargetYaw))
		return EMoveState::Rotating;
	
	float ShiftLen = MoveSpeed_ * InDelta;
	Approached = CurrentDist <= ShiftLen;
	
	if (Approached)
		ShiftLen = CurrentDist;

	FVector NewLoc = Current + Dir * ShiftLen;
	SetActorLocation(NewLoc, true);

	return Approached ? EMoveState::Approached : EMoveState::Moving;
}

bool ABaseUnitActor::UpdateRotation(float InDelta, float InTargetYaw)
{
	float CurrentYaw = CurrentYaw = FMath::UnwindDegrees(GetActorRotation().Yaw);
	float DeltaAngle = FMath::FindDeltaAngleDegrees(CurrentYaw, InTargetYaw);
		
	if (FMath::Abs(DeltaAngle) < KINDA_SMALL_NUMBER)
	{
		//MG_LOG(ABaseUnitActorLog, TEXT("CurrentYaw: %f; InTargetYaw: %f; DeltaAngle: %f"), CurrentYaw, InTargetYaw, DeltaAngle);
		return false;
	}
	
	const float NewYaw = FMath::FixedTurn(CurrentYaw, InTargetYaw, RotateSpeed_ * InDelta);
	//MG_LOG(ABaseUnitActorLog, TEXT("CurrentYaw: %f; InTargetYaw: %f; NewYaw: %f"), CurrentYaw, InTargetYaw, NewYaw);
	
	const FRotator WorldRot(0.f, NewYaw, 0.f);
	SetActorRotation(WorldRot);
	
	return true;
}


bool ABaseUnitActor::SetMoveTarget(const FRepAxialCoord& InTarget, const FAxialAngle& InAxialAngle)
{
	MG_LOG(ABaseUnitActorLog, TEXT("InTarget: %s; InAxialAngle: %s"), *InTarget.ToString(), *InAxialAngle.ToString());
	
	if (!HasAuthority())
	{
		return false;
	}
	
	Path_.Reset();
	
	if (InTarget == AxialCoord_)
	{
		bOnMove_ = InAxialAngle.R != AxialAngle_.R;
	}
	else
	{
		UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
		
		const FUnitDescription* Descr = GetDescription();
		RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Descr != nullptr);
		
		FVector Current = GetActorLocation();
	
		GridWorldSubsystem->FindPath(AxialCoord_.ToNative(), AxialAngle_.R, InTarget.ToNative(), InAxialAngle.R, 
			Descr->MaxRoundDistance, Descr->MoveCost, Descr->RotationCost, Current.Z, Path_, false);
	
		if (!Path_.IsEmpty())
		{
			RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Path_.Num() > 1);
			bOnMove_ = true;
		}
	}
			
	if (bOnMove_)
	{
		AxialCoord_ = InTarget;
		AxialAngle_ = InAxialAngle;
		
		PathIndex_ = 0;
		
		MG_LOG(ABaseUnitActorLog, TEXT("Target: %s; InTargetYaw: %f"), *InTarget.ToNative().ToString(), AxialAngle_.GetYaw());
	}
	
	return bOnMove_;
}

const FUnitDescription* ABaseUnitActor::GetDescription() const
{
	RETURN_ON_FAIL_NULL(ABaseUnitActorLog, !UnitType_.IsNone());

	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	return Content->GetDescription<FUnitDescription>(UnitType_);
}

void ABaseUnitActor::OnRep_UnitType()
{
	
}
