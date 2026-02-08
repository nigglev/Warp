// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Warp/Utils/RepAxialCoord.h"

class ABaseUnitActor;

namespace UnitActorFactory
{
	ABaseUnitActor* CreateUnitActor(const UObject* InWorldContext, FName InUnitType, const FTransform& InTransform, AActor* InOwnerActor = nullptr);
	ABaseUnitActor* CreateUnitActor(const UObject* InWorldContext, FName InUnitType, const HexMath::FAxialCoord& InAxialCoord);
};