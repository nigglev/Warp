// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneratedProcMeshActor.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"

AGeneratedProcMeshActor::AGeneratedProcMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh_ = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh_);

	ProcMesh_->bUseAsyncCooking = true; // полезно, если будешь делать collision на рантайме
}

void AGeneratedProcMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	BuildPolygon(CircumRadius_, SegmentCount_, FVector2D(GapPosition_, GapWidth_), *ProcMesh_);

	if (Material_)
	{
		//ProcMesh_->SetMaterial(0, Material_);
		
		UMaterialInstanceDynamic* MID = ProcMesh_->CreateDynamicMaterialInstance(0, Material_);
		MID->SetVectorParameterValue(TEXT("LineColor"), LineColor_);
		MID->SetScalarParameterValue(TEXT("GlowIntensity"), GlowIntensity_);
		MID->SetVectorParameterValue(TEXT("FillColor"), FillColor_);
		MID->SetScalarParameterValue(TEXT("FillIntensity"), FillIntensity_);
	}
}

namespace
{
	void AddVertex(FVector InPos, FVector Dir, FVector2D UV, FLinearColor InColor,
		TArray<FVector>& OutBaseVertices, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents, 
		TArray<FLinearColor>& Colors, TArray<FVector2D>& UV0)
	{
		OutBaseVertices.Add(InPos);
		Normals.Add(FVector::UnitZ());
		Tangents.Add(FProcMeshTangent(Dir, false));
		Colors.Add(InColor);
		UV0.Add(UV);
	}
	
	void AddCorner(FVector InCenter, double InCircumRadius, double InAngle, FVector2D InGap, int32 InIndex,
		TArray<FVector>& OutBaseVertices, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents, 
		TArray<FLinearColor>& Colors, TArray<FVector2D>& UV0)
	{
		FRotator Rotation(0, InIndex * -InAngle, 0);
		FVector Dir = Rotation.RotateVector(FVector::UnitX());
				
		double k = InGap.X - InGap.Y / 2;
		double l = (1 - k) * 0.5;
		int32 BaseU = InIndex & 1;
		if (BaseU == 1)
		{
			l = 1 - l;
		}
		
		FVector Point1 = InCenter + Dir * InCircumRadius * k;
		
		AddVertex(Point1, Dir, FVector2D(l, k), FLinearColor::Red, OutBaseVertices, Normals, Tangents,Colors, UV0);
		
		k = InGap.X + InGap.Y / 2;
		l = (1 - k) * 0.5;
		BaseU = InIndex & 1;
		if (BaseU == 1)
		{
			l = 1 - l;
		}
		
		FVector Point2 = InCenter + Dir * InCircumRadius * k;
		
		AddVertex(Point2, Dir, FVector2D(l, k), FLinearColor::Green, OutBaseVertices, Normals, Tangents,Colors, UV0);
		
		FVector Point3 = InCenter + Dir * InCircumRadius;
		
		AddVertex(Point3, Dir, FVector2D(BaseU, 1), FLinearColor::Blue, OutBaseVertices, Normals, Tangents,Colors, UV0);
	}
}

void AGeneratedProcMeshActor::BuildPolygon(double InCircumRadius, uint16 InSegmentCount, FVector2D InGap, UProceduralMeshComponent& OutProcMesh)
{
	if (!ensure(InCircumRadius > 0))
		return;
	if (!ensure(InSegmentCount >= 3))
		return;
	
	OutProcMesh.ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FLinearColor> Colors;
	TArray<FVector2D> UV0;
	
	double Angle = 360.f / InSegmentCount;
	
	FVector Center = FVector(0, 0, 0);
	AddVertex(Center, FVector::UnitX(), FVector2D(0.5, 0), FLinearColor::Red, Vertices, Normals, Tangents,Colors, UV0);
	
	AddCorner(Center, InCircumRadius, Angle, InGap, 0, Vertices, Normals, Tangents, Colors, UV0);
	
	for (int32 i = 1; i <= InSegmentCount; i++)
	{
		AddCorner(Center, InCircumRadius, Angle, InGap, i, Vertices, Normals, Tangents, Colors, UV0);
		
		Triangles.Add(0); //all times center
		int32 LastBaseIndex = Vertices.Num() - 1 - 2;
		Triangles.Add(LastBaseIndex - 3);
		Triangles.Add(LastBaseIndex);
		
		int32 base = 1;
		int32 InnerPrev = (i - 1) * 3 + base + 1;
		int32 OuterPrev = InnerPrev + 1;
		
		int32 InnerCur = i * 3 + base + 1;
		int32 OuterCur  = InnerCur + 1;
	
		Triangles.Add(InnerPrev); Triangles.Add(OuterPrev); Triangles.Add(InnerCur);
		Triangles.Add(OuterPrev); Triangles.Add(OuterCur);  Triangles.Add(InnerCur);
	}
	
	OutProcMesh.CreateMeshSection_LinearColor(
		0,
		Vertices,
		Triangles,
		Normals,
		UV0,
		Colors,
		Tangents,
		false
	);

	OutProcMesh.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}