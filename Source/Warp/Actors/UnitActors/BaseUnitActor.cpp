// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "AbilitySystemComponent.h"
#include "HexPathfainer.h"
#include "MGLogs.h"
#include "Net/UnrealNetwork.h"
#include "UnitCharacteristics/GAS/UnitStandardAttributeSet.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/HexMap/HexMapWS.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"
#include "Warp/Base/GameState/TurnMachine.h"

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
	
	AbilitySystemComponent_ = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent_->SetIsReplicated(true);

	AttributeSet_ = CreateDefaultSubobject<UUnitStandardAttributeSet>(TEXT("UnitStandardAttributeSet"));
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

FString ABaseUnitActor::GetDebugName() const
{
	if (UnitType_.IsNone())
		return FString::Printf(TEXT("NoneUnitType_%u"), GetUniqueID());
	return FString::Printf(TEXT("%s_%u"), *UnitType_.ToString(), GetUniqueID());
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
	
	CollectMaterials();
	InitAbilitySystemComponent();
}

void ABaseUnitActor::CollectMaterials()
{
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

void ABaseUnitActor::InitAbilitySystemComponent()
{
	RETURN_ON_FAIL(ABaseUnitActorLog, AbilitySystemComponent_ != nullptr);
	RETURN_ON_FAIL(ABaseUnitActorLog, AttributeSet_ != nullptr);
	
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL(ABaseUnitActorLog, Descr != nullptr);
	
	AbilitySystemComponent_->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		AttributeSet_->InitMaxHealth(Descr->MaxHealth);
		AttributeSet_->InitHealth(Descr->MaxHealth);

		AttributeSet_->InitMaxMovementPoints(Descr->MoveParams.MaxDistance);
		AttributeSet_->InitMovementPoints(Descr->MoveParams.MaxDistance);
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

FMoveParams ABaseUnitActor::GetCurrentMoveParams() const
{
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, Descr != nullptr, {});
	
	FMoveParams MoveParams = Descr->MoveParams;
	MoveParams.MaxDistance = GetMovementPoints();
	
	return MoveParams;
}

bool ABaseUnitActor::SetMoveTarget(const FAxialTransform& InTarget)
{
	MG_LOG(ABaseUnitActorLog, TEXT("%s; InTarget: %s"), *GetDebugName(), *InTarget.ToString());
	
	if (!HasAuthority())
	{
		return false;
	}
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, HexMapWS != nullptr);
	
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Descr != nullptr);
	
	bool bCapturable = HexMapWS->CaptureCells(GetUniqueID(), GetUniqueID(), InTarget.Position.ToNative(), InTarget.Rotation.R, Descr->Footprint, true);
	if (!bCapturable)
		return false;
	
	Path_.Reset();
	
	FMoveParams MoveParams = GetCurrentMoveParams();
	
	if (InTarget.Position == AxialTransform_.Position)
	{
		float MoveCost = HexMath::GetRotationDiff(AxialTransform_.Rotation.R, InTarget.Rotation.R) * MoveParams.RotationCost;
		if (!SpendMovementPoints(MoveCost))
		{
			return false;
		}
		
		bOnMove_ = InTarget.Rotation != AxialTransform_.Rotation;
		Path_.Emplace(InTarget.Position.ToNative(), InTarget.Rotation.R);
	}
	else
	{
		HexMapWS->FindPath(GetUniqueID(), AxialTransform_.Position.ToNative(), AxialTransform_.Rotation.R, 
			InTarget.Position.ToNative(), InTarget.Rotation.R, 
			MoveParams, Path_, false);
	
		if (!Path_.IsEmpty())
		{
			RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, Path_.Num() > 1);
			
			float MoveCost = Path_.Last().Distance;
			if (!SpendMovementPoints(MoveCost))
			{
				return false;
			}
			
			bOnMove_ = true;
		}
	}
			
	if (bOnMove_)
	{
		AxialTransform_ = InTarget;
		if (HasAuthority())
			OnRep_AxialTransform();
		
		PathIndex_ = 0;
		
		MG_LOG(ABaseUnitActorLog, TEXT("%s; Target: %s"), *GetDebugName(), *InTarget.ToString());
	}
	
	return bOnMove_;
}

const FUnitDescription* ABaseUnitActor::GetDescription() const
{
	RETURN_ON_FAIL_NULL(ABaseUnitActorLog, !UnitType_.IsNone());

	UWarpContentSubSystem* Content = UWarpContentSubSystem::Get(this);
	return Content->GetDescription<FUnitDescription>(UnitType_);
}

float ABaseUnitActor::GetHealth() const
{
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, AttributeSet_ != nullptr, 0.0f);
	 return AttributeSet_->GetHealth();
}

float ABaseUnitActor::GetMaxHealth() const
{
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, AttributeSet_ != nullptr, 0.0f);
	return AttributeSet_->GetMaxHealth();
}

float ABaseUnitActor::GetMovementPoints() const
{
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, AttributeSet_ != nullptr, 0.0f);
	return AttributeSet_->GetMovementPoints();
}

float ABaseUnitActor::GetMaxMovementPoints() const
{
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, AttributeSet_ != nullptr, 0.0f);
	return AttributeSet_->GetMaxMovementPoints();
}


void ABaseUnitActor::OnRep_UnitType()
{
	
}

void ABaseUnitActor::OnRep_AxialTransform()
{
	if (!Ghost_)
		CapturingHexes();
}

bool ABaseUnitActor::CanSpendMovementPoints(float InCost) const
{
	if (InCost <= 0)
	{
		return true;
	}

	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, AttributeSet_ != nullptr);

	return GetMovementPoints() + KINDA_SMALL_NUMBER >= InCost;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool ABaseUnitActor::SpendMovementPoints(float InCost)
{
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, HasAuthority());
	RETURN_ON_FAIL_BOOL(ABaseUnitActorLog, AttributeSet_ != nullptr);

	if (InCost <= 0)
	{
		return true;
	}

	const float CurrentPoints = AttributeSet_->GetMovementPoints();
	
	if (CurrentPoints + KINDA_SMALL_NUMBER < InCost)
	{
		MG_LOG(ABaseUnitActorLog, TEXT("%s; Not enough movement points. Current=%.1f Cost=%.1f"), *GetDebugName(),CurrentPoints, InCost);
		return false;
	}
	

	const float NewPoints = FMath::Clamp( CurrentPoints - InCost, 0.0f, AttributeSet_->GetMaxMovementPoints());

	MG_LOG(ABaseUnitActorLog, TEXT("%s; Current=%.1f Cost=%.1f NewPoints=%.1f"), *GetDebugName(), CurrentPoints, InCost, NewPoints);
	
	AttributeSet_->SetMovementPoints(NewPoints);
	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABaseUnitActor::RestoreMovementPoints()
{
	RETURN_ON_FAIL(ABaseUnitActorLog, HasAuthority());
	RETURN_ON_FAIL(ABaseUnitActorLog, AttributeSet_ != nullptr);
	
	MG_LOG(ABaseUnitActorLog, TEXT("%s"), *GetDebugName());
	
	AttributeSet_->SetMovementPoints(AttributeSet_->GetMaxMovementPoints());
}

void ABaseUnitActor::OnNewRound(uint32 InRoundNumber)
{
	RETURN_ON_FAIL(ABaseUnitActorLog, HasAuthority());
	MG_LOG(ABaseUnitActorLog, TEXT("%s; InRoundNumber: %u"), *GetDebugName(), InRoundNumber);
	
	RestoreMovementPoints();
}

int32 ABaseUnitActor::GetMovePriority() const
{
	const FUnitDescription* Descr = GetDescription();
	RETURN_ON_FAIL_DEFAULT(ABaseUnitActorLog, Descr != nullptr, 10000);
	
	return Descr->MovePriority;
}