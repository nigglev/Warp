// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnitActor.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Misc/MapErrors.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(ABaseUnitActorLog, Log, All);

// Sets default values
ABaseUnitActor::ABaseUnitActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ABaseUnitActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseUnitActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseUnitActor, UnitID);
}


