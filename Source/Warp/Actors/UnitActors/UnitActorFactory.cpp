// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitActorFactory.h"

#include "BaseUnitActor.h"
#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "Warp/ContentManagement/GameAssets.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(UnitFactoryLog, Log, All);

AActor* UnitActorFactory::CreateActor(const UObject* InWorldContext, const TSubclassOf<AActor>& InActorClass, 
	const HexMath::FAxialCoord& InAxialCoord, AActor* InOwner /*= nullptr*/)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, InWorldContext);
	
	TOptional<FVector> PosOpt = UHexGridWorldSubsystem::AxialCellToWorldCoord(InAxialCoord);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, PosOpt.IsSet());
	
	FActorSpawnParameters Params;
	Params.Owner = InOwner;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UWorld* World = InWorldContext->GetWorld();

	FVector SpawnLocation = PosOpt.GetValue();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	AActor* UnitActor = World->SpawnActor(InActorClass, &SpawnLocation, &SpawnRotation, Params);
	
	return UnitActor;
}

ABaseUnitActor* UnitActorFactory::CreateUnitActor(const UObject* InWorldContext, FName InUnitType,
	const HexMath::FAxialCoord& InAxialCoord, AActor* InOwner /*= nullptr*/, bool InGhost /*= false*/)
{
	RETURN_ON_FAIL_NULL(UnitFactoryLog, InWorldContext);
	
	TSubclassOf<ABaseUnitActor> UnitActorClass = UGameAssets::Get()->GetUnitActorClass(InUnitType, InGhost);
	RETURN_ON_FAIL_NULL(UnitFactoryLog, UnitActorClass);
	
	ABaseUnitActor* Unit = Cast<ABaseUnitActor>(CreateActor(InWorldContext, UnitActorClass, InAxialCoord, InOwner));
	RETURN_ON_FAIL_NULL(UnitFactoryLog, Unit);
	
	if (Unit != nullptr)
	{
		Unit->SetUnitType(InUnitType);
		Unit->SetAxialCoord(InAxialCoord);
	}
	
	return Unit;
}
