// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCharacteristics/UnitSize.h"
#include "Warp/Utils/AxialAngle.h"
#include "Warp/Utils/RepAxialCoord.h"
#include "BaseUnitActor.generated.h"

struct FUnitDescription;
struct FAxialAngle;

UCLASS()
class WARP_API ABaseUnitActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseUnitActor();
	virtual void BeginPlay() override;
	virtual void Tick(float InDelta) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool IsLoaded() const;

	FVector GetUnitWorldPosition() const {return GetActorLocation();}
	void SetUnitWorldPosition(const FVector& InWorldPosition) {SetActorLocation(InWorldPosition);}

	FName GetUnitType() const {return UnitType_;}
	void SetUnitType(const FName InUnitType) { UnitType_ = InUnitType; }
	
	void SetAxialCoord(const HexMath::FAxialCoord& InAxialCoord) { AxialCoord_ = InAxialCoord; }
	HexMath::FAxialCoord GetAxialCoord() const { return AxialCoord_.ToNative(); }
	
	FAxialAngle GetAxialAngle() const { return AxialAngle_; }
	
	bool SetMoveTarget(const FRepAxialCoord& InTarget, const FAxialAngle& InAxialAngle);
	
	bool IsMoving() const { return bOnMove_; }
	
	const FUnitDescription* GetDescription() const;
	
protected:
	UFUNCTION()
	void OnRep_UnitType();
	
	enum class EMoveState : uint8 { Moving, Rotating, Approached };	
	EMoveState MoveToTarget(float InDelta, const FVector& Target);
	
	bool UpdateRotation(float InDelta, float InTargetYaw);
	
	UPROPERTY(EditDefaultsOnly, Category="Move")
	float MoveSpeed_ = 600.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Move")
	float RotateSpeed_ = 360;

	UPROPERTY(EditDefaultsOnly, Category="Move")
	float AcceptanceRadius_ = 25.f;
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitType)
	FName UnitType_;
	
	UPROPERTY(Replicated)
	FRepAxialCoord AxialCoord_;
	
	UPROPERTY(Replicated)
	FAxialAngle AxialAngle_;
	
	UPROPERTY(Replicated)
	bool bOnMove_ = false;
	
	TArray<FVector> Path_;
	int32 PathIndex_ = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root_;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh_;
};
