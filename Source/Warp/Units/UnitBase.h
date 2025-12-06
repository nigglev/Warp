// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitEnums.h"
#include "UnitPosition.h"
#include "UnitRotation.h"
#include "UnitSize.h"
#include "UObject/Object.h"
#include "UnitBase.generated.h"

struct FUnitDefinition;
struct FUnitRecord;
class ADefaultPlayerController;
struct FCombatMapTile;
/**
 * 
 */

USTRUCT()
struct FEssentialInfoForUnitActor
{
	GENERATED_BODY()
	
	uint32 UnitCombatID;
	FUnitSize UnitSize;
	FIntVector2 UnitGridPosition;
	FUnitRotation UnitRotation;
	
};

UCLASS()
class WARP_API UUnitBase : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	static UUnitBase* CreateUnit(UObject* Outer, const FUnitDefinition* InUnitDefinition, const uint32 InUnitCombatID, const EUnitAffiliation InUnitAffiliation);

	void Init(const FUnitDefinition* InUnitDefinition, const uint32 InUnitCombatID, const EUnitAffiliation InUnitAffiliation);

	FName GetUnitTypeName() const {return UnitTypeName_;}
	uint32 GetUnitCombatID() const {return UnitCombatID_;}
	EUnitAffiliation GetUnitAffiliation() const {return UnitAffiliation_;}
	FEssentialInfoForUnitActor GetUnitActorEssentialInfo() const;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitPosition)
	FUnitPosition UnitPosition;
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitRotation)
	FUnitRotation UnitRotation;
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UnitSize)
	FUnitSize UnitSize_;

	int32 GetSpeed() const {return Speed_;}
	int32 GetMaxAP() const {return MaxAP_;}
	int32 GetCurrentAP	() const {return CurrentAP_;}

	FString ToString() const;
	bool TryParseUnitSizeEnum(const FString& InString, EUnitSizeCategory& OutValue);
	
protected:
	UFUNCTION()
	void OnRep_UnitTypeName();
	UFUNCTION()
	void OnRep_CombatUnitID();
	UFUNCTION()
	void OnRep_UnitAffiliation();
	UFUNCTION()
	void OnRep_UnitPosition();
	UFUNCTION()
	void OnRep_UnitRotation();
	UFUNCTION()
	void OnRep_UnitSize();
	
	UFUNCTION()
	void OnRep_UnitSpeed();
	UFUNCTION()
	void OnRep_UnitMaxAP();
	UFUNCTION()
	void OnRep_UnitCurrentAP();
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitTypeName)
	FName UnitTypeName_;
	UPROPERTY(ReplicatedUsing=OnRep_CombatUnitID)
	uint32 UnitCombatID_;
	UPROPERTY(ReplicatedUsing=OnRep_UnitAffiliation)
	EUnitAffiliation UnitAffiliation_;
	
	UPROPERTY(ReplicatedUsing=OnRep_UnitSpeed)
	int32 Speed_ = 10;
	UPROPERTY(ReplicatedUsing=OnRep_UnitMaxAP)
	int32 MaxAP_ = 2;
	UPROPERTY(ReplicatedUsing=OnRep_UnitCurrentAP)
	int32 CurrentAP_ = 0;

};
