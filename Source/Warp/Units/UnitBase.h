// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnitBase.generated.h"

class ADefaultPlayerController;
struct FCombatMapTile;
/**
 * 
 */
UENUM(BlueprintType)
enum class EUnitAffiliation : uint8
{
	PlayerControlled UMETA(DisplayName="Player"),
	EnemyAI          UMETA(DisplayName="Enemy AI"),
	AllyAI           UMETA(DisplayName="Ally AI")
};

UENUM(BlueprintType)
enum class EUnitRotation : uint8
{
	Rot_0 = 0	UMETA(DisplayName="0°"),
	Rot_45		UMETA(DisplayName="45°"),
	Rot_90		UMETA(DisplayName="90°"),
	Rot_135		UMETA(DisplayName="135°"),
	Rot_180		UMETA(DisplayName="180°"),
	Rot_225		UMETA(DisplayName="225°"),
	Rot_270		UMETA(DisplayName="270°"),
	Rot_315		UMETA(DisplayName="315°"),
	Max			UMETA(DisplayName="MAX")
};

UENUM(BlueprintType)
enum class EUnitSizeCategory : uint8
{
	None   UMETA(DisplayName="None"),
	Small  UMETA(DisplayName="Small (1×1)"),
	Medium UMETA(DisplayName="Medium (2×1)"),
	Big    UMETA(DisplayName="Big (3×1)")
};

USTRUCT()
struct FUnitPosition
{
	GENERATED_BODY()

	FIntVector2 GetUnitTilePosition() const {return FIntVector2(UnitTileX, UnitTileY);}
	FVector2f GetUnitWorldPosition() const { return UnitWorldPosition; }

	void SetUnitPosition(const FVector2f& InWorldPosition, const FIntVector2& InTilePosition) {UnitWorldPosition = InWorldPosition; UnitTileX = InTilePosition.X; UnitTileY = InTilePosition.Y;}
	void SetUnitTilePosition(const FIntVector2& InTilePosition) {UnitTileX = InTilePosition.X; UnitTileY = InTilePosition.Y;}
	void SetUnitWorldPosition(const FVector2f& InWorldPosition) {UnitWorldPosition = InWorldPosition;}
	
private:	
	UPROPERTY()
	uint32 UnitTileX = UINT32_MAX;
	UPROPERTY()
	uint32 UnitTileY = UINT32_MAX;
	UPROPERTY()
	FVector2f UnitWorldPosition = FVector2f::ZeroVector;
};

USTRUCT()
struct FUnitRotation
{
	GENERATED_BODY()

	FUnitRotation() = default;
	explicit FUnitRotation(const EUnitRotation In) : UnitRotation(In) {}
	
	static FUnitRotation Max()   { return FUnitRotation(EUnitRotation::Max); }
	static FUnitRotation Rot0()  { return FUnitRotation(EUnitRotation::Rot_0); }
	static FUnitRotation Rot45() { return FUnitRotation(EUnitRotation::Rot_45); }
	static FUnitRotation Rot90()    { return FUnitRotation(EUnitRotation::Rot_90); }
	static FUnitRotation Rot35()   { return FUnitRotation(EUnitRotation::Rot_135); }
	static FUnitRotation Rot180()  { return FUnitRotation(EUnitRotation::Rot_180); }
	static FUnitRotation Rot225() { return FUnitRotation(EUnitRotation::Rot_225); }
	static FUnitRotation Rot270()    { return FUnitRotation(EUnitRotation::Rot_270); }
	static FUnitRotation Rot315()    { return FUnitRotation(EUnitRotation::Rot_315); }
	
	void RotateClockwise()
	{
		constexpr uint8 Count = static_cast<uint8>(EUnitRotation::Max);
		const uint8 Cur   = static_cast<uint8>(UnitRotation) % Count;
		UnitRotation      = static_cast<EUnitRotation>((Cur + 1) % Count);
	}

	void RotateCounterClockwise()
	{
		constexpr uint8 Count = static_cast<uint8>(EUnitRotation::Max);
		const uint8 Cur   = static_cast<uint8>(UnitRotation) % Count;
		UnitRotation      = static_cast<EUnitRotation>((Cur + Count - 1) % Count);
	}
		
	EUnitRotation GetUnitRotation() const {return UnitRotation;}
	float GetUnitFRotation () const
	{
		const uint8 i = static_cast<uint8>(UnitRotation);
		return (i < static_cast<uint8>(EUnitRotation::Max)) ? i * 45.f : 0.f;
	}

	void SetUnitRotation(const EUnitRotation InUnitRotation) {UnitRotation = InUnitRotation;}
	void SetUnitRotation(const float InUnitRotation)
	{
		float Norm = FMath::Fmod(InUnitRotation, 360.f);
		if (Norm < 0.f)
			Norm += 360.f;
		const int32 Step = FMath::RoundToInt(Norm / 45.f) % 8;
		UnitRotation = static_cast<EUnitRotation>(1 + Step);
	}
	
private:	
	UPROPERTY()
	EUnitRotation UnitRotation = EUnitRotation::Max;
};

USTRUCT()
struct FUnitSize
{
	GENERATED_BODY()

	FUnitSize() = default;
	explicit FUnitSize(const EUnitSizeCategory In) : SizeCategory(In) {}
	
	static FUnitSize None()   { return FUnitSize(EUnitSizeCategory::None); }
	static FUnitSize Small()  { return FUnitSize(EUnitSizeCategory::Small); }
	static FUnitSize Medium() { return FUnitSize(EUnitSizeCategory::Medium); }
	static FUnitSize Big()    { return FUnitSize(EUnitSizeCategory::Big); }
	
	EUnitSizeCategory GetUnitSize() const {return SizeCategory;}
	FIntVector2 GetUnitTileLength() const
	{
		switch (SizeCategory)
		{
			case EUnitSizeCategory::None:	return FIntVector2(0,0);
			case EUnitSizeCategory::Small:  return FIntVector2(1,1);
			case EUnitSizeCategory::Medium: return FIntVector2(3,1);
			case EUnitSizeCategory::Big:    return FIntVector2(5,1);
			default:                        return FIntVector2(0,0);
		}
	}
	void SetUnitSize(const EUnitSizeCategory InSizeCategory) {SizeCategory = InSizeCategory;}
	
private:	
	UPROPERTY()
	EUnitSizeCategory SizeCategory = EUnitSizeCategory::None;
};


UCLASS()
class WARP_API UUnitBase : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	static UUnitBase* CreateUnit(UObject* Outer, const uint32 InUnitID,
									const FUnitSize&  InSizeCategory, const FUnitRotation& InUnitRotation);

	void SetOwningPlayer(APlayerState* InOwner);
	void InitStats(EUnitAffiliation InAffiliation, int32 InSpeed, int32 InMaxAP)
	{
		Affiliation = InAffiliation;
		Speed = InSpeed;
		MaxAP = InMaxAP;
		CurrentAP = 0;
	}

	void StartNewTurn() { CurrentAP = MaxAP; }
	bool SpendAP(const int32 Amount) { if (Amount <= 0) return false; if (CurrentAP < Amount) return false; CurrentAP -= Amount; return true; }
	bool HasAnyAffordableAction(const int32 MinActionCost) const { return CurrentAP >= MinActionCost; }

	APlayerState* GetOwningPlayer() const {return OwningPlayer;}
	uint32 GetUnitID() const {return UnitID;}
	int32 GetSpeed() const {return Speed;}
	EUnitAffiliation GetAffiliation() const {return Affiliation;}
	int32 GetMaxAP() const {return MaxAP;}
	int32 GetCurrentAP	() const {return CurrentAP;}
	FString ToString() const;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitPosition)
	FUnitPosition UnitPosition;
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitRotation)
	FUnitRotation UnitRotation;
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitSize)
	FUnitSize UnitSize;
	
protected:
	UFUNCTION()
	void OnRep_OwningPlayer();
	UFUNCTION()
	void OnRep_UnitID();
	UFUNCTION()
	void OnRep_UnitPosition();
	UFUNCTION()
	void OnRep_UnitRotation();
	UFUNCTION()
	void OnRep_UnitSize();

	UFUNCTION()
	void OnRep_UnitAffiliation();
	UFUNCTION()
	void OnRep_UnitSpeed();
	UFUNCTION()
	void OnRep_UnitMaxAP();
	UFUNCTION()
	void OnRep_UnitCurrentAP();

	UPROPERTY(ReplicatedUsing=OnRep_OwningPlayer)
	TObjectPtr<APlayerState> OwningPlayer = nullptr;
	UPROPERTY(ReplicatedUsing=OnRep_UnitID)
	uint32 UnitID;

	UPROPERTY(ReplicatedUsing=OnRep_UnitAffiliation)
	EUnitAffiliation Affiliation = EUnitAffiliation::EnemyAI;
	UPROPERTY(ReplicatedUsing=OnRep_UnitSpeed)
	int32 Speed = 10;
	UPROPERTY(ReplicatedUsing=OnRep_UnitMaxAP)
	int32 MaxAP = 2;
	UPROPERTY(ReplicatedUsing=OnRep_UnitCurrentAP)
	int32 CurrentAP = 0;

};
