// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "UnitMeshRow.generated.h"

USTRUCT(BlueprintType)
struct FUnitMeshRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FName UnitType = NAME_None;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> Mesh;
};