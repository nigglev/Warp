// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "HexPathfainer.h"
#include "MGLogs.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/HexMap/HexMapWS.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"
#include "Warp/TurnBasedSystem/TurnMachine.h"

DEFINE_LOG_CATEGORY_STATIC(ABaseUnitActorLog, Log, All);

namespace
{
	FName HoverOpacityName(TEXT("HoverOpacity"));
}

ABaseUnitActor::ABaseUnitActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	Root_ = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root_);

	Mesh_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh_->SetupAttachment(Root_);
	
	Mesh_->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh_->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ABaseUnitActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseUnitActor, UnitType_);
	DOREPLIFETIME(ABaseUnitActor, bOnMove_);
	DOREPLIFETIME(ABaseUnitActor, AxialTransform_);
}

bool ABaseUnitActor::IsLoaded() const
{
	return UnitType_ != NAME_None;
}

void ABaseUnitActor::Init(const FName InUnitType, const FAxialTransform& InAxialTransform, bool InGhost)
{
	UnitType_ = InUnitType;
	Ghost_ = InGhost;
	
	AxialTransform_ = InAxialTransform;
	if (HasAuthority())
		OnRep_AxialTransform();
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
	
	const int32 MaterialCount = Mesh_->GetNumMaterials();
	DynamicMaterials_.Reserve(MaterialCount);

	for (int32 i = 0; i < MaterialCount; ++i)
	{
		UMaterialInterface* BaseMat = Mesh_->GetMaterial(i);
		if (!BaseMat)
		{
			DynamicMaterials_.Add(nullptr);
			continue;
		}

		UMaterialInstanceDynamic* MID = Mesh_->CreateDynamicMaterialInstance(i, BaseMat);
		DynamicMaterials_.Add(MID);

		if (MID)
		{
			MID->SetScalarParameterValue(HoverOpacityName, NormalOpacity_);
		}
	}
}

void ABaseUnitActor::NotifyActorBeginCursorOver()
{
	Super::NotifyActorBeginCursorOver();
	MG_FUNC_LABEL(ABaseUnitActorLog);
	
	AWarpGameState* WGS = Cast<AWarpGameState>(GetWorld()->GetGameState());
	RETURN_ON_FAIL(ABaseUnitActorLog, WGS != nullptr);
	
	if (WGS->GetTurnMachine()->GetActiveUnit() == this)
	{
		MG_LOG(ABaseUnitActorLog, TEXT("Active Unit"));
		
		SetShipOpacity(HoverOpacity_);
	}
}

void ABaseUnitActor::SetShipOpacity(float InOpacity)
{
	for (UMaterialInstanceDynamic* MID : DynamicMaterials_)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(HoverOpacityName, InOpacity);
		}
	}
}

void ABaseUnitActor::NotifyActorEndCursorOver()
{
	MG_FUNC_LABEL(ABaseUnitActorLog);
	
	AWarpGameState* WGS = Cast<AWarpGameState>(GetWorld()->GetGameState());
	RETURN_ON_FAIL(ABaseUnitActorLog, WGS != nullptr);
	
	if (WGS->GetTurnMachine()->GetActiveUnit() == this)
	{
		MG_LOG(ABaseUnitActorLog, TEXT("Active Unit"));
		
		SetShipOpacity(NormalOpacity_);
	}
	
	Super::NotifyActorEndCursorOver();
}

void ABaseUnitActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SetOnStartTimerHandle_.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SetOnStartTimerHandle_);
		SetOnStartTimerHandle_.Invalidate();
	}
	
	Super::EndPlay(EndPlayReason);
}

void ABaseUnitActor::Tick(float InDelta)
{
	Super::Tick(InDelta);

	if (!HasAuthority() || !bOnMove_)
	{
		return;
	}
	
	FVector Current = GetActorLocation();
	
	if (Path_.IsValidIndex(PathIndex_))
	{
		HexMath::FPathNode& Node = Path_[PathIndex_];
		
		TOptional<FVector> TargetPosOpt = UHexMapWS::AxialCellToWorldCoord(Node.Coord, Current.Z);
		RETURN_ON_FAIL(ABaseUnitActorLog, TargetPosOpt.IsSet());
		
		FVector Target = TargetPosOpt.GetValue();
		
		EMoveState MoveState = MoveToTarget(InDelta, Target);
		if (MoveState == EMoveState::Approached)
		{
			PathIndex_++;
		}
		return;
	}
	
	
	float TargetYaw  = FMath::UnwindDegrees(AxialTransform_.Rotation.GetYaw());
	bOnMove_ = UpdateRotation(InDelta, TargetYaw);
	
	if (bOnMove_)
		return;
	
	if (Ghost_)
	{
		const FGameplayDescription* Descr = UWarpContentSubSystem::GetGameplayDescription(this);
		RETURN_ON_FAIL(ABaseUnitActorLog, Descr);
		
		GetWorld()->GetTimerManager().SetTimer(SetOnStartTimerHandle_, this, &ABaseUnitActor::SetOnStartPathPoint, Descr->GhostDelayTime);
	}
	else
	{
		CapturingHexes();
		
		auto GS = Cast<AWarpGameState>(GetWorld()->GetGameState());
		RETURN_ON_FAIL(ABaseUnitActorLog, GS);
		GS->OnUnitArrived.Broadcast(this);
	}	
}

void ABaseUnitActor::SetOnStartPathPoint()
{
	RETURN_ON_FAIL(ABaseUnitActorLog, !Path_.IsEmpty());
	
	PathIndex_ = 0;
	
	FVector Current = GetActorLocation();
				
	TOptional<FVector> TargetPosOpt = UHexMapWS::AxialCellToWorldCoord(Path_[0].Coord, Current.Z);
	RETURN_ON_FAIL(ABaseUnitActorLog, TargetPosOpt.IsSet());
				
	const FRotator WorldRot(0.f, FAxialAngle::GetYaw(Path_[0].Rotation), 0.f);
				
	SetActorLocationAndRotation(TargetPosOpt.GetValue(), WorldRot);
	
	bOnMove_ = true;
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

	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, Descr != nullptr, EMoveState::Approached);
	
	float ShiftLen = Descr->AnimationMoveSpeed * InDelta;
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
	
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Descr != nullptr);
	
	const float NewYaw = FMath::FixedTurn(CurrentYaw, InTargetYaw, Descr->AnimationRotationSpeed * InDelta);
	//MG_LOG(ABaseUnitActorLog, TEXT("CurrentYaw: %f; InTargetYaw: %f; NewYaw: %f"), CurrentYaw, InTargetYaw, NewYaw);
	
	const FRotator WorldRot(0.f, NewYaw, 0.f);
	SetActorRotation(WorldRot);
	
	return true;
}

void ABaseUnitActor::CapturingHexes()
{
	if (Ghost_)
		return;
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	RETURN_ON_FAIL(ABaseUnitActorLog, HexMapWS != nullptr);
		
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL(ABaseUnitActorLog, Descr != nullptr);
	
	HexMapWS->CaptureCells(GetUniqueID(), GetUniqueID(), AxialTransform_.Position.ToNative(), AxialTransform_.Rotation.R, Descr->Footprint, false);
}

bool ABaseUnitActor::SetCirclePath(TArray<HexMath::FPathNode>&& InPath)
{
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, !InPath.IsEmpty());
	
	if (SetOnStartTimerHandle_.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SetOnStartTimerHandle_);
		SetOnStartTimerHandle_.Invalidate();
	}
	
	Path_ = MoveTemp(InPath);
	
	AxialTransform_.Position = Path_.Last().Coord;
	AxialTransform_.Rotation = FAxialAngle(Path_.Last().Rotation);
	if (HasAuthority())
		OnRep_AxialTransform();
	
	SetOnStartPathPoint();
	
	return true;
}

void ABaseUnitActor::SetLastRotation(FAxialAngle InAxialAngle)
{
	RETURN_ON_FAIL(ABaseUnitActorLog, !Path_.IsEmpty());
	
	Path_.Last().Rotation = InAxialAngle.R;
	AxialTransform_.Rotation = InAxialAngle;
}

bool ABaseUnitActor::SetMoveTarget(const FAxialTransform& InTarget)
{
	MG_LOG(ABaseUnitActorLog, TEXT("InTarget: %s"), *InTarget.ToString());
	
	if (!HasAuthority())
	{
		return false;
	}
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Descr != nullptr);
	
	bool bCapturable = HexMapWS->CaptureCells(GetUniqueID(), GetUniqueID(), InTarget.Position.ToNative(), InTarget.Rotation.R, Descr->Footprint, true);
	if (!bCapturable)
		return false;
	
	Path_.Reset();
	
	if (InTarget.Position == AxialTransform_.Position)
	{
		bOnMove_ = InTarget.Rotation != AxialTransform_.Rotation;
		Path_.Emplace(InTarget.Position.ToNative(), InTarget.Rotation.R);
	}
	else
	{
		HexMapWS->FindPath(GetUniqueID(), AxialTransform_.Position.ToNative(), AxialTransform_.Rotation.R, 
			InTarget.Position.ToNative(), InTarget.Rotation.R, 
			Descr->MoveParams, Path_, false);
	
		if (!Path_.IsEmpty())
		{
			RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Path_.Num() > 1);
			bOnMove_ = true;
		}
	}
			
	if (bOnMove_)
	{
		AxialTransform_ = InTarget;
		if (HasAuthority())
			OnRep_AxialTransform();
		
		PathIndex_ = 0;
		
		MG_LOG(ABaseUnitActorLog, TEXT("Target: %s"), *InTarget.ToString());
	}
	
	return bOnMove_;
}

const FUnitDescription* ABaseUnitActor::GetDescription() const
{
	RETURN_ON_FAIL_NULL(ABaseUnitActorLog, !UnitType_.IsNone());

	UWarpContentSubSystem* Content = UWarpContentSubSystem::Get(this);
	return Content->GetDescription<FUnitDescription>(UnitType_);
}

void ABaseUnitActor::OnRep_UnitType()
{
	
}

void ABaseUnitActor::OnRep_AxialTransform()
{
	if (!Ghost_)
		CapturingHexes();
}
