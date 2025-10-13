// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Warp/CombatMap/CombatMap.h"
#include "CombatMapManager.generated.h"

enum class EUnitActorState : uint8;
struct FNetMovePath;
enum class EUnitSizeCategory : uint8;
enum class EUnitRotation : uint8;
class ABaseUnitActor;
class AWarpGameState;
class UUnitBase;


UCLASS()
class WARP_API ACombatMapManager : public AActor
{
	GENERATED_BODY()

public:
	ACombatMapManager();
	virtual void BeginPlay() override;
	
	void Init(uint32 InGridSize, uint32 InTileSize);
	void GenerateGrid();

	ABaseUnitActor* SpawnUnitActorGhost(const FVector2f& InUnitPos, const FUnitSize& InUnitSize,
		const FUnitRotation& InUnitRotation);
	ABaseUnitActor* SpawnUnitActorGhost(const ABaseUnitActor* InBasedOnUnitActor);
	void SpawnUnit(const uint32 InUnitID, const FVector2f& InUnitPos, const FUnitSize& InUnitSize,
		const FUnitRotation& InUnitRotation, EUnitActorState InUnitActorState);
	void DestroyGhostActor(ABaseUnitActor* InGhostActor);
	bool IsPositionForUnitAvailable(const FIntPoint& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const;

	void UpdateHoveredTile(int32 NewInstanceIndex);
	void UpdateVisualForBlockers(const TArray<FIntPoint>& InBlockers);
	
	AWarpGameState* GetGameState() const;
	UInstancedStaticMeshComponent* GetTilesISMC() const {return TilesISMC;}
	uint32 GetGridSize()   const { return GridSize; }
	uint32 GetTileSizeUU() const { return TileSize; }
	int32 GetGridSquare() const { return GridSize * GridSize; }

	int32 GridToTileIndex(const FIntPoint& InGridPos) const;
	FIntPoint TileInstanceToGridPosition(int32 TileInstanceIndex) const;
	FVector GridToLevelPosition(const FIntPoint& TileGridCoords) const;
	FVector CombatMapToLevelPosition(const FVector2f& InPos) const;
	
protected:

	void SetupLayers() const;
	void BuildBaseInstances();
	
	UFUNCTION()
	void SpawnUnitActors(const TArray<UUnitBase*>& ReplicatedUnits);
	

	void SetBlocker(int32 TileIdx);
	void ClearBlocker() const;
	void SetHover(int32 TileIdx);
	void ClearHover() const;

	UPROPERTY()
	FVector GridOrigin;
	UPROPERTY()
	uint32 GridSize;
	UPROPERTY()
	uint32 TileSize;
	UPROPERTY()
	TArray<FTransform> TilesTransformsLocal;
	UPROPERTY()
	TMap<uint32, ABaseUnitActor*> UnitActorsByID;

	int32 CurrentHoveredInstance = INDEX_NONE;
	bool IsInitialUnitsSpawned = false;
	const float MetresToUUScale = 100.0f;
	float UnitZOffset = 50.f;
	float BlockerZLift = 1.0f;
	float PreviewZLift = 0.5f;
	float MoveTileZLift = 0.3f;
	float HighlightZLift = 0.1f;
	bool bLayersSetup = false;
	
	UPROPERTY()
	const APlayerController* OwnerPC;

	UPROPERTY(EditAnywhere, Category="Unit")
	TSubclassOf<ABaseUnitActor> UnitActorClass;

	UPROPERTY(VisibleAnywhere, Category="Grid")
	UInstancedStaticMeshComponent* TilesISMC = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Grid|Overlay")
	UInstancedStaticMeshComponent* HoverISMC = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Grid|Overlay")
	UInstancedStaticMeshComponent* PreviewISMC = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Grid|Overlay")
	UInstancedStaticMeshComponent* BlockedISMC = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Grid|Overlay")
	UInstancedStaticMeshComponent* MovementISMC = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UStaticMesh* TileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UMaterialInterface* BaseMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UMaterialInterface* HoverMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UMaterialInterface* PreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UMaterialInterface* BlockedMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="Grid|Assets")
	UMaterialInterface* MovementMaterial = nullptr;
};


