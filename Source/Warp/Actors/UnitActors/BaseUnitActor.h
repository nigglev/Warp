// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexPathfainer.h"
#include "GameFramework/Actor.h"
#include "Warp/Utils/RepAxialCoord.h"
#include "AbilitySystemInterface.h"
#include "BaseUnitActor.generated.h"

struct FUnitDescription;
struct FAxialAngle;
class UAbilitySystemComponent;
class UUnitStandardAttributeSet;

UCLASS()
class WARP_API ABaseUnitActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseUnitActor();
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float InDelta) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool IsLoaded() const;

	void Init(const FName InUnitType, const FAxialTransform& InAxialTransform, bool InGhost);
	
	FVector GetUnitWorldPosition() const {return GetActorLocation();}
	void SetUnitWorldPosition(const FVector& InWorldPosition) {SetActorLocation(InWorldPosition);}

	FName GetUnitType() const {return UnitType_;}
	
	HexMath::FAxialCoord GetAxialPosition() const { return AxialTransform_.Position.ToNative(); }
	
	FAxialAngle GetAxialRotation() const { return AxialTransform_.Rotation; }
	
	bool SetCirclePath(TArray<HexMath::FPathNode>&& InPath);
	
	void SetLastRotation(FAxialAngle InAxialAngle);
	
	bool SetMoveTarget(const FAxialTransform& InTarget);
	
	bool IsMoving() const { return bOnMove_; }
	
	const FUnitDescription* GetDescription() const;
	
	FMoveParams GetCurrentMoveParams() const;
	
	// GAS
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent_; }

	UFUNCTION(BlueprintCallable, Category="Ship|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category="Ship|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category="Ship|Attributes")
	float GetMovementPoints() const;

	UFUNCTION(BlueprintCallable, Category="Ship|Attributes")
	float GetMaxMovementPoints() const;
	// GAS End
protected:
	UFUNCTION()
	void OnRep_UnitType();
	
	UFUNCTION()
	void OnRep_AxialTransform();
	
	void CollectMaterials();
	void InitAbilitySystemComponent();
	
	enum class EMoveState : uint8 { Moving, Rotating, Approached };	
	EMoveState MoveToTarget(float InDelta, const FVector& Target);
	
	bool UpdateRotation(float InDelta, float InTargetYaw);
	
	void SetOnStartPathPoint();
	
	void CapturingHexes();
	
	void SetShipOpacity(float InOpacity);
	
	bool CanSpendMovementPoints(float InCost) const;

	bool SpendMovementPoints(float InCost);

	void RestoreMovementPoints();
	
	UPROPERTY(EditAnywhere, Category="Hover")
	float HoverOpacity_ = 0.35f;

	UPROPERTY(EditAnywhere, Category="Hover")
	float NormalOpacity_ = 1.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitType)
	FName UnitType_;
	
	UPROPERTY(ReplicatedUsing=OnRep_AxialTransform)
	FAxialTransform AxialTransform_;
	
	UPROPERTY(Replicated)
	bool bOnMove_ = false;
	
	TArray<HexMath::FPathNode> Path_;
	int32 PathIndex_ = 0;
	
	bool Ghost_ = false;
	
	FTimerHandle SetOnStartTimerHandle_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root_;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh_;
	
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMaterials_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Unit|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent_ = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Unit|GAS")
	TObjectPtr<UUnitStandardAttributeSet> AttributeSet_ = nullptr;
};
