// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Warp/Utils/AxialAngle.h"
#include "PlacePointer.generated.h"

UCLASS()
class WARP_API APlacePointer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlacePointer();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
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
};
