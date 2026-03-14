// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexPathfainer.h"
#include "GameFramework/Actor.h"
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
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// Called every frame
	virtual void Tick(float InDeltaTime) override;
	
	HexMath::FAxialCoord GetCoord() const { return PathNode_.Coord; }
	FAxialAngle GetAxialAngle() const { return AxialAngle_; }
	
	TOptional<FAxialTransform> GetCapturedTransform(bool IbClear) const;
	
	void Set(TArray<HexMath::FPathNode>&& InPath, ABaseUnitActor* InActiveUnit);
	
	void FixRotation();	
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	bool IsCaptured() const { return bCaptured_; }

protected:
	
	void TryChangeAngle();
	void UpdateRotation(float InDelta);
	
	void FixRotation(bool InFixed);
	
	void OnTransformChanged();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pointer Parameters")
	float DeadZone_ = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pointer Parameters")
	float RotateSpeed_ = 360;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pointer Parameters")
	FLinearColor StartColor_ = FLinearColor::Green;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pointer Parameters")
	FLinearColor FixedColor_ = FLinearColor::Yellow;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pointer Parameters")
	bool CreateGhost_ = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh_;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> ArrowMesh_;
	
	UPROPERTY()
	UMaterialInstanceDynamic* RingMat_;
	
	UPROPERTY()
	UMaterialInstanceDynamic* ArrowMat_;
	
	FAxialAngle AxialAngle_;
	
	HexMath::FPathNode PathNode_;
	
	UPROPERTY()
	ABaseUnitActor* ActiveUnit_;
	
	bool bRotationFixed_ = false;
	
	UPROPERTY()
	ABaseUnitActor* Ghost_;
	
	bool bCaptured_ = false;
};
