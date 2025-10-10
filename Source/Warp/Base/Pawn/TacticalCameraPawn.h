// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TacticalCameraPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class WARP_API ATacticalCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ATacticalCameraPawn();
	virtual void Tick(float DeltaSeconds) override;

	void FocusOn(const FVector& TargetWorld, float Duration = 0.35f);
	void AddZoom(float AxisValue);
	void MoveXY(const FVector2D& Axis, float DeltaSeconds);
	void AddRotation(float Degrees);
	void SetLockedToTarget(bool bLock, const FVector& InitialTarget = FVector::ZeroVector);
	void SetFollowTarget(const FVector& TargetWorld);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class USceneComponent* YawRoot;

	// --- Panning (FocusOn) ---
	bool   bIsPanning = false;
	float  PanTime = 0.f;
	float  PanElapsed = 0.f;
	FVector PanStart = FVector::ZeroVector;
	FVector PanTarget = FVector::ZeroVector;

	// --- Zoom/Move tuning ---
	float MinArmLength = 800.f;
	float MaxArmLength = 4000.f;
	float ZoomSpeed = 200.f;
	float MoveSpeedUU = 2200.f;

	// --- Lock/follow ---
	bool   bLockedToTarget = false;
	FVector FollowTarget = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category="Camera|Follow")
	float FollowLerpSpeed = 6.0f;

};
