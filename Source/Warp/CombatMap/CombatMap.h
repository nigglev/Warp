// CombatMap.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Warp/Units/UnitBase.h"
#include "CombatMap.generated.h"

enum class EUnitRotation : uint8;
enum class EUnitSizeCategory : uint8;

USTRUCT()
struct FCombatMapTile
{
	GENERATED_BODY()
	
	UPROPERTY()
	uint32 X = 0;
	UPROPERTY()
	uint32 Y = 0;
	UPROPERTY()
	FVector2f WorldPosition = FVector2f::ZeroVector;
	UPROPERTY()
	uint32 PlacedUnitID = 0;

	bool operator==(const FCombatMapTile& Other) const
	{
		return X==Other.X && Y==Other.Y;
	}
};

FORCEINLINE uint32 GetTypeHash(const FCombatMapTile& Tile)
{
	return HashCombine(::GetTypeHash(Tile.X), ::GetTypeHash(Tile.Y));
}

class UUnitBase;

UCLASS()
class WARP_API UCombatMap : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PlaceUnitAt(UUnitBase* InUnitToPlace, const FIntPoint& InCenter);
	void PlaceUnitOnMapRand(UUnitBase* InUnitToPlace);
	void RemoveUnitFromMap(UUnitBase* InUnitToRemove);

	FCombatMapTile MakeTile(int32 X, int32 Y) const;


	uint32 GetGridSize() const {return GridSize;}
	uint32 GetTileSize() const {return TileSize;}

	bool CheckPositionForUnit(const FIntPoint& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const;

protected:
	virtual void PostInitProperties() override;
	void InitializeFreeTiles();
	
	FCombatMapTile GetRandomPlaceableTile(const FUnitSize& InUnitSize, const FUnitRotation& InUnitRotation, TArray<FCombatMapTile>& OutFootprint) const;
	bool ExtractFootprint(const FCombatMapTile& InCenterTile, const FUnitSize& InUnitSize, const FUnitRotation& InUnitRotation,
		TArray<FCombatMapTile>& OutFootprint, TArray<FIntPoint>* OutBlockers = nullptr) const;
	void SetupUnitPosOnMap(UUnitBase* InUnitToPlace, const FCombatMapTile& InUnitCenter, const TArray<FCombatMapTile>& InFootprint);
	
	void ModifyTilesOccupancy(const TArray<FCombatMapTile>& Tiles, const UUnitBase* Unit, bool bAdd);
	static FIntVector2 GetMovementDelta(const EUnitRotation InUnitRotation);
	static void LogFootprint(const TArray<FCombatMapTile>& InFootprint, bool IsFreed);

	FCombatMapTile MakeTileWithUnitID(int32 X, int32 Y, uint8 InUnitID) const;
	
	UFUNCTION()
	void OnRep_GridSize();
	UFUNCTION()
	void OnRep_TileSize();
	UFUNCTION()
	void OnRep_OccupiedTiles();
	
	UPROPERTY(ReplicatedUsing=OnRep_GridSize)
	uint32 GridSize = 20;
	UPROPERTY(ReplicatedUsing=OnRep_TileSize)
	uint32 TileSize = 4;
	UPROPERTY(ReplicatedUsing=OnRep_OccupiedTiles)
	TArray<FCombatMapTile> OccupiedTiles;
	UPROPERTY()
	TArray<FCombatMapTile> FreeTiles;
};
