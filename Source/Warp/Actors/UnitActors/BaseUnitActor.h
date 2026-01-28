// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCharacteristics/UnitSize.h"
#include "BaseUnitActor.generated.h"

UENUM()
enum class EUnitActorState : uint8
{
	None = 0,
	Playable = 1,
	Ghost = 2,
	MAX
};

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

	FUnitSize GetUnitActorSize() const {return UnitActorSize;}
	void SetUnitActorSize(const FUnitSize InSize) {UnitActorSize = InSize;}

	void SetMoveTarget(const FVector& InTarget);
	
protected:
	virtual void PostNetInit() override;
	UFUNCTION()
	void OnRep_UnitActorSize();
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitActorSize)
	FUnitSize UnitActorSize = FUnitSize::None();
	
	UPROPERTY(EditDefaultsOnly, Category="Move")
	float MoveSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category="Move")
	float AcceptanceRadius = 25.f;

	UPROPERTY(Replicated)
	FVector_NetQuantize10 MoveTarget = FVector::ZeroVector;

	UPROPERTY(Replicated)
	bool bHasMoveTarget = false;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
