// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnitActorFactory.generated.h"

struct FUnitDataTableRows;
struct FUnitDescription;
class ABaseUnitActor;
class UWarpPlayfabContentSubSystem;
/**
 * 
 */
USTRUCT()
struct FUnitActorFactoryConfig
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UDataTable> UnitsTable = nullptr;

	UPROPERTY()
	ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
};

UCLASS()
class WARP_API UUnitActorFactory : public UObject
{
	GENERATED_BODY()

public:
	void Init(UWarpPlayfabContentSubSystem* InContent, const FUnitActorFactoryConfig& InConfig);
	
	ABaseUnitActor* CreateFromRow(const FUnitDataTableRows& InRow, const FTransform& InTransform, AActor* InOwnerActor = nullptr);
	ABaseUnitActor* Create(TSubclassOf<ABaseUnitActor> InUnitClass, const FUnitDescription& InDesc, const FTransform& InTransform, AActor* InOwnerActor = nullptr);

protected:
	TSubclassOf<ABaseUnitActor> ResolveClass(const FUnitDataTableRows& InRow) const;
	static void ApplyDescription(ABaseUnitActor& Actor, const FUnitDescription& Desc);
	
	UPROPERTY()
	TObjectPtr<UWarpPlayfabContentSubSystem> Content_ = nullptr;
	UPROPERTY()
	FUnitActorFactoryConfig Config_;

};