// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitActorFactory.h"

#include "BaseUnitActor.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/UnitStaticData/UnitDataTableRows.h"

DEFINE_LOG_CATEGORY_STATIC(UnitFactoryLog, Log, All);

ABaseUnitActor* UUnitActorFactory::CreateByDTData(const FUnitDataTableRows& InRow, const FTransform& InTransform, AActor* InOwnerActor)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, !InRow.UnitType.IsNone());
	RETURN_ON_FAIL_NULL(UnitFactoryLog, GetWorld());
	
	UWorld* World = GetWorld();
	
	TSubclassOf<ABaseUnitActor> UnitClass = ResolveClass(InRow);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitClass);
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = InOwnerActor;

	ABaseUnitActor* UnitActor = World->SpawnActor<ABaseUnitActor>(UnitClass, InTransform, Params);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitActor);

	UnitActor->SetUnitType(InRow.UnitType);
	return UnitActor;
}

TSubclassOf<ABaseUnitActor> UUnitActorFactory::ResolveClass(const FUnitDataTableRows& InRow) const
{
	UClass* Loaded = InRow.UnitActor.LoadSynchronous();
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Loaded);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Loaded->IsChildOf(ABaseUnitActor::StaticClass()));
	return Loaded;
}
