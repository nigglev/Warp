// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitActorFactory.h"

#include "BaseUnitActor.h"
#include "Warp/ContentManagement/GameAssets.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(UnitFactoryLog, Log, All);

ABaseUnitActor* UnitActorFactory::CreateUnitActor(const UObject* InWorldContext, FName InUnitType, const FTransform& InTransform, AActor* InOwnerActor)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, InWorldContext);

	TSubclassOf<ABaseUnitActor> UnitActorClass = UGameAssets::Get()->GetUnitActorClass(InUnitType);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitActorClass);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = InOwnerActor;

	UWorld* World = InWorldContext->GetWorld();

	ABaseUnitActor* UnitActor = World->SpawnActor<ABaseUnitActor>(UnitActorClass, InTransform, Params);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitActor);

	UnitActor->SetUnitType(InUnitType);
	return UnitActor;
}