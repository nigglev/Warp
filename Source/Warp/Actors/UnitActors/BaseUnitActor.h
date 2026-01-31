// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCharacteristics/UnitSize.h"
#include "BaseUnitActor.generated.h"

UCLASS()
class WARP_API ABaseUnitActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseUnitActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FVector GetUnitWorldPosition() const {return GetActorLocation();}
	void SetUnitWorldPosition(const FVector& InWorldPosition) {SetActorLocation(InWorldPosition);}

	FName GetUnitType() const {return UnitType_;}
	void SetUnitType(const FName InUnitType) {UnitType_ = InUnitType;}	
	
	FUnitSize GetUnitActorSize() const {return UnitActorSize_;}
	void SetUnitActorSize(const FUnitSize InSize) {UnitActorSize_ = InSize;}

	void SetMoveTarget(const FVector& InTarget);
	bool IsMoving() const { return bHasMoveTarget_; }
	
protected:
	UFUNCTION()
	void OnRep_UnitType();
	UFUNCTION()
	void OnRep_UnitActorSize();

	UPROPERTY(ReplicatedUsing=OnRep_UnitType)
	FName UnitType_ = FName("Unit");
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitActorSize)
	FUnitSize UnitActorSize_ = FUnitSize::None();
	
	UPROPERTY(EditDefaultsOnly, Category="Move")
	float MoveSpeed_ = 600.f;

	UPROPERTY(EditDefaultsOnly, Category="Move")
	float AcceptanceRadius_ = 25.f;

	UPROPERTY(Replicated)
	FVector_NetQuantize10 MoveTarget_ = FVector::ZeroVector;

	UPROPERTY(Replicated)
	bool bHasMoveTarget_ = false;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root_;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh_;
};
