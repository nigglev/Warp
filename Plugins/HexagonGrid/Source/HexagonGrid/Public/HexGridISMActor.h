#pragma once

#include "CoreMinimal.h"
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
	
	void SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, bool InSelected);
	void SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType);

protected:
	virtual void BeginPlay() override;
	
	void BuildHexagon(uint32 Radius);
	
	void SetColor(int32 InIndex, const FLinearColor InColor, float InZOffset) const;
	
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
	float ZOffset_ = 0.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float SizeScale_ = 1.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	bool HexRotation_ = false;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float FadeStartRadius_ = 500.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float FadeLength_ = 500.f;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	FLinearColor SelectedColor_ = FLinearColor::Gray;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float SelectedZOffset_ = 0;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	FLinearColor Colors_[static_cast<int32>(ECellType::MAX_VALUE)] ={
		FLinearColor::Green,
		FLinearColor::Black,
		FLinearColor::Yellow,
	};
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float ZOffsets_[static_cast<int32>(ECellType::MAX_VALUE)] ={
		-2,
		0,
		2,
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

	struct FSelectStatus
	{
		ECellType BaseStatus = ECellType::Opened;
		bool bSelected = false;
	};
	
	void ChangeCellStatus(int32 InIndex, ECellType InCellType);
	void ChangeSelectStatus(int32 InIndex, bool InSelected);
	
	void SetCellType(int32 InIndex, ECellType InCellType);
	void SetSelectStatus(int32 InIndex, bool InSelected);
	
	FLinearColor GetColor(int32 InIndex) const;
	float GetZOffset(int32 InIndex) const;
	
	TMap<int32, FSelectStatus> SelectStatus_;
};