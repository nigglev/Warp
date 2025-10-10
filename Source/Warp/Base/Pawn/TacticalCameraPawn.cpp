// Fill out your copyright notice in the Description page of Project Settings.


#include "TacticalCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


ATacticalCameraPawn::ATacticalCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	YawRoot = CreateDefaultSubobject<USceneComponent>(TEXT("YawRoot"));
	RootComponent = YawRoot;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(YawRoot);
	
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll  = false;
	SpringArm->bInheritYaw   = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATacticalCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsPanning)
	{
		PanElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(PanElapsed / PanTime, 0.f, 1.f);
		const float Smooth = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.0f);
		const FVector NewLoc = FMath::Lerp(PanStart, PanTarget, Smooth);
		SetActorLocation(NewLoc);

		if (Alpha >= 1.f) bIsPanning = false;
	}
	
	if (bLockedToTarget && !FollowTarget.IsNearlyZero())
	{
		const FVector Curr = GetActorLocation();
		const FVector Goal = FVector(FollowTarget.X, FollowTarget.Y, Curr.Z);
		const FVector NewLoc = FMath::VInterpTo(Curr, Goal, DeltaSeconds, FollowLerpSpeed);
		SetActorLocation(NewLoc);
	}
}


void ATacticalCameraPawn::FocusOn(const FVector& TargetWorld, float Duration)
{
	PanStart  = GetActorLocation();
	PanTarget = FVector(TargetWorld.X, TargetWorld.Y, PanStart.Z);
	PanElapsed = 0.f;
	PanTime    = FMath::Max(0.01f, Duration);
	bIsPanning = true;

	if (bLockedToTarget)
		FollowTarget = PanTarget;
}

void ATacticalCameraPawn::AddZoom(float AxisValue)
{
	if (FMath::IsNearlyZero(AxisValue))
		return;
	const float NewLen = FMath::Clamp(SpringArm->TargetArmLength - AxisValue * ZoomSpeed, MinArmLength, MaxArmLength);
	SpringArm->TargetArmLength = NewLen;
}

void ATacticalCameraPawn::MoveXY(const FVector2D& Axis, float DeltaSeconds)
{
	if (bLockedToTarget)
		return;
	if (Axis.IsNearlyZero())
		return;
	
	FVector Fwd   = YawRoot->GetForwardVector();
	Fwd.Z = 0.f; Fwd.Normalize();
	
	FVector Right = YawRoot->GetRightVector();
	Right.Z = 0.f; Right.Normalize();

	const FVector Delta = (Fwd * Axis.Y + Right * Axis.X) * MoveSpeedUU * DeltaSeconds;
	SetActorLocation(GetActorLocation() + Delta);
}

void ATacticalCameraPawn::AddRotation(float Degrees)
{
	if (FMath::IsNearlyZero(Degrees)) return;
	YawRoot->AddLocalRotation(FRotator(0.f, Degrees, 0.f));
}

void ATacticalCameraPawn::SetLockedToTarget(bool bLock, const FVector& InitialTarget)
{
	bLockedToTarget = bLock;
	if (bLockedToTarget)
	{
		FollowTarget = FVector(InitialTarget.X, InitialTarget.Y, GetActorLocation().Z);
		FocusOn(FollowTarget, 0.25f);
	}
}

void ATacticalCameraPawn::SetFollowTarget(const FVector& TargetWorld)
{
	FollowTarget = FVector(TargetWorld.X, TargetWorld.Y, GetActorLocation().Z);
}

