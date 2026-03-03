#pragma once

#include "CoreMinimal.h"
#include "CellLayers.h"
#include "ECellType.h"
#include "HexMath.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexGridISMActor.generated.h"

namespace HexMathOffset
{
	enum class EHexOffsetLayout : uint8;
}

UCLASS()
class HEXAGONGRID_API AHexGridISMActor : public AActor
{
	GENERATED_BODY()

public:
	AHexGridISMActor();
	
	FVector GetExtent() const;
	
	float GetHexSize() const { return HexSize_; }
	uint32 GetGridSize() const { return GridSize_; }

	virtual void OnConstruction(const FTransform& Transform) override;
	
	void SetChunkCoord(const HexMath::FOffsetCoord& InChunkCoord) { ChunkCoord_ = InChunkCoord; }
	
	void SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel = 1);

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	virtual void BeginPlay() override;

protected:
	void BuildHexagon(uint32 Radius);
	
	void SetHexColor(int32 InIndex, const FLinearColor& InColor) const;
	void SetHexZOffset(int32 InIndex, float InZOffset) const;
	
	UFUNCTION(CallInEditor, Category="Grid")
	void UpdateMPC();
	
	UFUNCTION(CallInEditor, Category="Grid")
	void Rebuild();
	
	UPROPERTY(EditAnywhere, Category="Grid")
	TObjectPtr<UStaticMesh> HexTileMesh_;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	TObjectPtr<class UMaterialInterface> Material_ = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Grid", meta=(ClampMin="1.0"))
	float HexSize_ = 100.f; // центр -> вершина

	UPROPERTY(EditAnywhere, Category="Grid", meta=(ClampMin="2"))
	uint32 GridSize_ = 10;

	UPROPERTY(EditAnywhere, Category="Grid")
	float ZOffset_ = 0.2f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float SizeScale_ = 1.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	bool HexRotation_ = false;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float FadeStartRadius_ = 500.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float FadeLength_ = 500.f;
		
	UPROPERTY(EditAnywhere, Category="Grid")
	TMap<ECellType, FLinearColor> CellColors_ = {
		{ ECellType::Opened, FLinearColor::Black },
		{ ECellType::MoveProjection, FLinearColor::Blue },
		{ ECellType::Captured, FLinearColor::Red },
		{ ECellType::Closed, FLinearColor::Black },
		{ ECellType::Selected, FLinearColor::Yellow },
	};
		
	UPROPERTY(EditAnywhere, Category="Grid")
	double GlowIntensity_ = 1;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	bool DetailDebug_ = false;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float DebugBoundsTime_ = 60 * 60;
	
	UPROPERTY(VisibleAnywhere, Category="Grid")
	TObjectPtr<UInstancedStaticMeshComponent> ISM_;

	UPROPERTY(EditAnywhere, Category="Grid")
	TObjectPtr<UMaterialParameterCollection> MPC_;
	
	HexMath::FOffsetCoord ChunkCoord_;
	
	void ChangeCellStatus(int32 InIndex, ECellType InCellType, float InLevel);
	
	void SetCellType(int32 InIndex, ECellType InCellType, float InLevel);
	
	FLinearColor GetColor(int32 InIndex) const;
	float GetZOffset(int32 InIndex) const;
	
	TMap<int32, FCellLayers> SelectStatus_;
};