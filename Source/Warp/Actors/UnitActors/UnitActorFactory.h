// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Warp/Utils/RepAxialCoord.h"

class ABaseUnitActor;

namespace UnitActorFactory
{
	AActor* CreateActor(const UObject* InWorldContext, const TSubclassOf<AActor>& InActorClass, 
		const FAxialTransform& InAxialTransform, AActor* InOwner = nullptr);
	
	ABaseUnitActor* CreateUnitActor(const UObject* InWorldContext, FName InUnitType, const FAxialTransform& InAxialTransform, 
		AActor* InOwner = nullptr, bool InGhost = false);
};