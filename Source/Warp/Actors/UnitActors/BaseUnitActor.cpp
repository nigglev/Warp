// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "HexGridWorldSubsystem.h"
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
	
	FVector Current = GetActorLocation();
	
	if (!Path_.IsEmpty())
	{
		FVector Target = Path_[0];
		FVector Dir = Target - Current;
		float CurrentDist;
		Dir.ToDirectionAndLength(Dir, CurrentDist);
	
		float TargetYaw  = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		
		bRotating_ = UpdateRotation(InDelta, TargetYaw);
		if (!bRotating_)
		{
			float ShiftLen = MoveSpeed_ * InDelta;
			if (CurrentDist <= ShiftLen)
			{
				ShiftLen = CurrentDist;
				Path_.RemoveAt(0);
			}

			FVector NewLoc = Current + Dir * ShiftLen;

			SetActorLocation(NewLoc, true);
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
	
	float CurrentYaw = CurrentYaw = FMath::UnwindDegrees(GetActorRotation().Yaw);
	
	if (InTarget == AxialCoord_)
	{
		AxialAngle_ = InAxialAngle;
		float DeltaAngle = FMath::FindDeltaAngleDegrees(CurrentYaw, AxialAngle_.GetYaw());
		
		bOnMove_ = FMath::Abs(DeltaAngle) > KINDA_SMALL_NUMBER;
		
		if (bOnMove_)
		{
			MG_LOG(ABaseUnitActorLog, TEXT("CurrentYaw: %f; InTargetYaw: %f"), CurrentYaw, AxialAngle_.GetYaw());
		}
	}
	else
	{
		MG_COND_ERROR_SHORT(ABaseUnitActorLog, !Path_.IsEmpty());
		Path_.Reset();
	
		UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
	
		TArray<HexMath::FAxialCoord> Path;
		GridWorldSubsystem->FindPath(AxialCoord_.ToNative(), InTarget.ToNative(), Path);
	
		if (!Path.IsEmpty())
		{
			RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Path.Num() > 1);
			Path.RemoveAt(0);
		
			FVector Current = GetActorLocation();
			for (HexMath::FAxialCoord AC : Path)
			{
				TOptional<FVector> TargetPosOpt = UHexGridWorldSubsystem::AxialCellToWorldCoord(AC, Current.Z);
				if (TargetPosOpt.IsSet())
				{
					Path_.Add(TargetPosOpt.GetValue());
				}
			}
	
			AxialCoord_ = InTarget;
			AxialAngle_ = InAxialAngle;
	
			MG_LOG(ABaseUnitActorLog, TEXT("Target: %s; CurrentYaw: %f; InTargetYaw: %f"), *InTarget.ToNative().ToString(), CurrentYaw, AxialAngle_.GetYaw());
	
			bOnMove_ = true;
		}
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
