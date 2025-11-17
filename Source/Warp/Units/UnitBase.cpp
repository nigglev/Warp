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
	DOREPLIFETIME(UUnitBase, UnitTypeName_);
	DOREPLIFETIME(UUnitBase, UnitCombatID_);
	DOREPLIFETIME(UUnitBase, UnitAffiliation_);
	DOREPLIFETIME(UUnitBase, UnitSize);
	DOREPLIFETIME(UUnitBase, UnitPosition);
	DOREPLIFETIME(UUnitBase, UnitRotation);
	
	DOREPLIFETIME(UUnitBase, Speed_);
	DOREPLIFETIME(UUnitBase, MaxAP_);
	DOREPLIFETIME(UUnitBase, CurrentAP_);
}

auto UUnitBase::CreateUnit(UObject* Outer, const FName& InUnitTypeName, const uint32 InUnitCombatID, const EUnitAffiliation InUnitAffiliation,
	const EUnitSizeCategory InUnitSize) -> UUnitBase*
{
	UUnitBase* NewUnit = NewObject<UUnitBase>(Outer);
	NewUnit->UnitTypeName_ = InUnitTypeName;
	NewUnit->UnitCombatID_ = InUnitCombatID;
	NewUnit->UnitAffiliation_ = InUnitAffiliation;
	NewUnit->UnitSize = FUnitSize(InUnitSize);
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("New Unit Created: %s"), *NewUnit->ToString());
	return NewUnit;
}

void UUnitBase::InitStats(int32 InSpeed, int32 InMaxAP)
{
	Speed_ = InSpeed;
	MaxAP_ = InMaxAP;
	CurrentAP_ = 0;
}


FUnitActorEssentialInfo UUnitBase::GetUnitActorEssentialInfo() const
{
	FUnitActorEssentialInfo UnitActorEssentialInfo;
	UnitActorEssentialInfo.UnitCombatID = UnitCombatID_;
	UnitActorEssentialInfo.UnitSize = UnitSize;
	UnitActorEssentialInfo.UnitGridPosition = UnitPosition.GetUnitTilePosition();
	UnitActorEssentialInfo.UnitRotation = UnitRotation;
	return UnitActorEssentialInfo;
}

void UUnitBase::OnRep_UnitTypeName()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: UnitTypeName: %s"), *GetName(), *UnitTypeName_.ToString());
}

void UUnitBase::OnRep_CombatUnitID()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: UnitID: %d"), *GetName(), UnitCombatID_);
}

void UUnitBase::OnRep_UnitAffiliation()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: UnitAffiliation: %d"), *GetName(), UnitAffiliation_);
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

void UUnitBase::OnRep_UnitSpeed()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: Speed: %d"), *GetName(), Speed_);
}

void UUnitBase::OnRep_UnitMaxAP()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: MaxAP: %d"), *GetName(), MaxAP_);
}

void UUnitBase::OnRep_UnitCurrentAP()
{
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
	TEXT("Name = %s: CurrentAP: %d"), *GetName(), CurrentAP_);
}


FString UUnitBase::ToString() const
{
	return FString::Printf(
		TEXT("UUnitBase { Unit: ID = %d, Type = %s, Affiliation = %d, UnitSize = %d (%d, %d), UnitWorldPosition = (%f, %f), UnitGridPosition = (%u, %u), UnitRotationValue = (%f) }"),
		UnitCombatID_, *UnitTypeName_.ToString(), UnitAffiliation_,
		UnitSize.GetUnitSize(), UnitSize.GetUnitTileLength().X, UnitSize.GetUnitTileLength().Y,
		UnitPosition.GetUnitWorldPosition().X, UnitPosition.GetUnitWorldPosition().Y,
		UnitPosition.GetUnitTilePosition().X, UnitPosition.GetUnitTilePosition().Y,
		UnitRotation.GetUnitFRotation()
	);
}

