// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "UnitDataTableRows.generated.h"

USTRUCT(BlueprintType)
struct FUnitActorsTableRows : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<ABaseUnitActor> UnitActor;
};