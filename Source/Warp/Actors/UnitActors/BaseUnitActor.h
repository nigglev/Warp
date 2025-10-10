// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Warp/Units/UnitBase.h"
#include "BaseUnitActor.generated.h"

UCLASS()
class WARP_API ABaseUnitActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseUnitActor();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetHighlighted(bool bOn) const;
	void ResizeMeshToSize(uint32 TargetLength, uint32 TargetWidth) const;
	void ResizeMeshToSize(const FVector& InScale) const;

	uint32 GetID() const {return UnitID;}
	void SetID(const uint32 InID) {UnitID = InID;}
	
protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditAnywhere, Category="Highlight")
	UMaterialInterface* HighlightMaterial;
	UPROPERTY(VisibleAnywhere, Category="Highlight")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(Replicated)
	uint32 UnitID = 0;
};
