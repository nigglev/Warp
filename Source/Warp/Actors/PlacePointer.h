// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexPathfainer.h"
#include "GameFramework/Actor.h"
#include "Warp/Utils/AxialAngle.h"
#include "Warp/Utils/RepAxialCoord.h"
#include "PlacePointer.generated.h"

class ABaseUnitActor;

UCLASS()
class WARP_API APlacePointer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlacePointer();
	
	// Called every frame
	virtual void Tick(float InDeltaTime) override;
	
	HexMath::FPathNode GetPathNode() const { return PathNode_; }
	
	void Init(const HexMath::FPathNode& InPathNode, ABaseUnitActor* InActiveUnit);
	
	FAxialAngle GetAxialAngle() const { return AxialAngle_; }

protected:
	
	void TryChangeAngle();
	void UpdateRotation(float InDelta);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rotate Parameters")
	float DeadZone_ = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rotate Parameters")
	float RotateSpeed_ = 360;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> ArrowMesh_;
	
	FAxialAngle AxialAngle_;
	
	HexMath::FPathNode PathNode_;
	
	UPROPERTY()
	ABaseUnitActor* ActiveUnit_;
};
