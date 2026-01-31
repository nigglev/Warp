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

UCLASS()
class WARP_API UUnitActorFactory : public UObject
{
	GENERATED_BODY()

public:
	ABaseUnitActor* CreateByDTData(const FUnitDataTableRows& InRow, const FTransform& InTransform, AActor* InOwnerActor = nullptr);

protected:
	TSubclassOf<ABaseUnitActor> ResolveClass(const FUnitDataTableRows& InRow) const;
	static void ApplyDescription(ABaseUnitActor& Actor, const FUnitDescription& Desc);
};