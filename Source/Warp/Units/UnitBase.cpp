// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitBase.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
DEFINE_LOG_CATEGORY_STATIC(UUnitBaseLog, Log, All);

void UUnitBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUnitBase, OwningPlayer);
	DOREPLIFETIME(UUnitBase, UnitID);
	DOREPLIFETIME(UUnitBase, UnitSize);
	DOREPLIFETIME(UUnitBase, UnitPosition);
	DOREPLIFETIME(UUnitBase, UnitRotation);

	DOREPLIFETIME(UUnitBase, Affiliation);
	DOREPLIFETIME(UUnitBase, Speed);
	DOREPLIFETIME(UUnitBase, MaxAP);
	DOREPLIFETIME(UUnitBase, CurrentAP);
}

auto UUnitBase::CreateUnit(UObject* Outer, const uint32 InUnitID,
	const FUnitSize&  InSizeCategory, const FUnitRotation& InUnitRotation) -> UUnitBase*
{
	UUnitBase* NewUnit = NewObject<UUnitBase>(Outer);
	NewUnit->UnitID = InUnitID;
	NewUnit->UnitSize = InSizeCategory;
	NewUnit->UnitRotation = InUnitRotation;
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("New Unit Created: %s"), *NewUnit->ToString());
	return NewUnit;
}

void UUnitBase::SetOwningPlayer(APlayerState* InOwner)
{
	OwningPlayer = InOwner;
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Owner of Unit = %s is now {%s}"), *this->ToString(), *InOwner->GetName());
}

void UUnitBase::OnRep_OwningPlayer()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: OwningPlayer: %s"), *GetName(), *OwningPlayer->GetName());
}

void UUnitBase::OnRep_UnitID()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: UnitID: %d"), *GetName(), UnitID);
}

void UUnitBase::OnRep_UnitPosition()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("Name = %s: Unit Tile Position: %s, Unit Tile Position: %s"), *GetName(),
		*UnitPosition.GetUnitTilePosition().ToString(), *UnitPosition.GetUnitWorldPosition().ToString());
}

void UUnitBase::OnRep_UnitRotation()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("Name = %s: Unit Rotation: %f"), *GetName(), UnitRotation.GetUnitFRotation());
}

void UUnitBase::OnRep_UnitSize()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("Name = %s: Unit Size: %d, Unit Tile Length: %s"), *GetName(),
		UnitSize.GetUnitSize(), *UnitSize.GetUnitTileLength().ToString());
}

void UUnitBase::OnRep_UnitAffiliation()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: Affiliation: %d"), *GetName(), Affiliation);
}

void UUnitBase::OnRep_UnitSpeed()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: Speed: %d"), *GetName(), Speed);
}

void UUnitBase::OnRep_UnitMaxAP()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: MaxAP: %d"), *GetName(), MaxAP);
}

void UUnitBase::OnRep_UnitCurrentAP()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: CurrentAP: %d"), *GetName(), CurrentAP);
}

FString UUnitBase::ToString() const
{
	return FString::Printf(
		TEXT("UUnitBase { UnitID = %d, UnitSize = %d (%d, %d), UnitWorldPosition = (%f, %f), UnitGridPosition = (%u, %u), UnitRotationValue = (%f) }"),
		UnitID,
		UnitSize.GetUnitSize(), UnitSize.GetUnitTileLength().X, UnitSize.GetUnitTileLength().Y,
		UnitPosition.GetUnitWorldPosition().X, UnitPosition.GetUnitWorldPosition().Y,
		UnitPosition.GetUnitTilePosition().X, UnitPosition.GetUnitTilePosition().Y,
		UnitRotation.GetUnitFRotation()
	);
}

