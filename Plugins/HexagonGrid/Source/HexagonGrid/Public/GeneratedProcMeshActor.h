// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeneratedProcMeshActor.generated.h"

class UProceduralMeshComponent;

UCLASS()
class HEXAGONGRID_API AGeneratedProcMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AGeneratedProcMeshActor();
	
	double GetCircumRadius() const { return CircumRadius_; }
	
	int32 GetSegmentCount() const { return SegmentCount_; }
	
	double GetAngle() const { return 360.f / SegmentCount_; }
	
	double GetEdgeLength() const
	{
		double A = GetAngle();
		double SinValue = FMath::Sin(FMath::DegreesToRadians(A) / 2.);
		return 2 * CircumRadius_ * SinValue;
	}
	
	double GetInRadius() const
	{
		double E = GetEdgeLength();
		return FMath::Sqrt(CircumRadius_ * CircumRadius_ - E * E / 4);
	}


protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	static void BuildPolygon(double InCircumRadius, uint16 InSegmentCount, FVector2D InGap, UProceduralMeshComponent& OutProcMesh);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> ProcMesh_;

	UPROPERTY(EditAnywhere, Category="Gen")
	double CircumRadius_ = 100.f;
	
	UPROPERTY(EditAnywhere, Category="Gen", meta=(ClampMin="3", ClampMax="60"))
	int32 SegmentCount_ = 6;
	
	UPROPERTY(EditAnywhere, Category="Gen", meta=(ClampMin="0", ClampMax="1"))
	double GapPosition_ = 0.6;
	
	UPROPERTY(EditAnywhere, Category="Gen", meta=(ClampMin="0", ClampMax="1"))
	double GapWidth_ = 0.000001;

	UPROPERTY(EditAnywhere, Category="Gen")
	TObjectPtr<class UMaterialInterface> Material_ = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Gen")
	FLinearColor LineColor_ = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, Category="Gen")
	FLinearColor FillColor_ = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, Category="Gen")
    double GlowIntensity_ = 1;
	
	UPROPERTY(EditAnywhere, Category="Gen")
	double FillIntensity_ = 1;
};
