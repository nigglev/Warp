// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitActorFactory.h"

#include "BaseUnitActor.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/UnitStaticData/UnitDataTableRows.h"

DEFINE_LOG_CATEGORY_STATIC(UnitFactoryLog, Log, All);

void UUnitActorFactory::Init(UWarpPlayfabContentSubSystem* InContent, const FUnitActorFactoryConfig& InConfig)
{
	Content_ = InContent;
	Config_ = InConfig;
}

ABaseUnitActor* UUnitActorFactory::CreateFromRow(const FUnitDataTableRows& InRow, const FTransform& InTransform, AActor* InOwnerActor)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Content_);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Config_.UnitsTable);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, !InRow.UnitType.IsNone());

	const FUnitDescription& Desc = Content_->GetDescription<FUnitDescription>(InRow.UnitType);

	TSubclassOf<ABaseUnitActor> UnitClass = ResolveClass(InRow);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitClass);

	return Create(UnitClass, Desc, InTransform, InOwnerActor);
}

ABaseUnitActor* UUnitActorFactory::Create(TSubclassOf<ABaseUnitActor> InUnitClass, const FUnitDescription& InDesc, const FTransform& InTransform, AActor* InOwnerActor)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, *InUnitClass);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, GetWorld());
	
	UWorld* World = GetWorld();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = Config_.CollisionHandling;
	Params.Owner = InOwnerActor;

	ABaseUnitActor* UnitActor = World->SpawnActor<ABaseUnitActor>(InUnitClass, InTransform, Params);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitActor);

	ApplyDescription(*UnitActor, InDesc);
	return UnitActor;
}

TSubclassOf<ABaseUnitActor> UUnitActorFactory::ResolveClass(const FUnitDataTableRows& InRow) const
{
	UClass* Loaded = InRow.UnitActor.LoadSynchronous();
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Loaded);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Loaded->IsChildOf(ABaseUnitActor::StaticClass()));
	return Loaded;
}

void UUnitActorFactory::ApplyDescription(ABaseUnitActor& Actor, const FUnitDescription& Desc)
{
	FUnitSize Size(Desc.UnitSize);
	Actor.SetUnitActorSize(Size);
}