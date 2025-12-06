// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitBase.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Base/GameInstanceSubsystem/WarpPlayfabContentSubSystem.h"
DEFINE_LOG_CATEGORY_STATIC(UUnitBaseLog, Log, All);

void UUnitBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUnitBase, UnitTypeName_);
	DOREPLIFETIME(UUnitBase, UnitCombatID_);
	DOREPLIFETIME(UUnitBase, UnitAffiliation_);
	DOREPLIFETIME(UUnitBase, UnitSize_);
	DOREPLIFETIME(UUnitBase, UnitPosition);
	DOREPLIFETIME(UUnitBase, UnitRotation);
	
	DOREPLIFETIME(UUnitBase, Speed_);
	DOREPLIFETIME(UUnitBase, MaxAP_);
	DOREPLIFETIME(UUnitBase, CurrentAP_);
}

UUnitBase* UUnitBase::CreateUnit(UObject* Outer, const FUnitDefinition* InUnitDefinition, const uint32 InUnitCombatID, const EUnitAffiliation InUnitAffiliation)
{
	UUnitBase* NewUnit = NewObject<UUnitBase>(Outer);
	NewUnit->Init(InUnitDefinition, InUnitCombatID, InUnitAffiliation);
	
	MG_COND_LOG(UUnitBaseLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitBase),
		TEXT("New Unit Created: %s"), *NewUnit->ToString());
	
	return NewUnit;
}

void UUnitBase::Init(const FUnitDefinition* InUnitDefinition, const uint32 InUnitCombatID,
	const EUnitAffiliation InUnitAffiliation)
{
	UnitCombatID_ = InUnitCombatID;
	UnitAffiliation_ = InUnitAffiliation;
	UnitTypeName_ = InUnitDefinition->UnitTypeName;
	EUnitSizeCategory UnitSizeCategory;
	if (TryParseUnitSizeEnum(InUnitDefinition->UnitSize, UnitSizeCategory))
		UnitSize_ = FUnitSize(UnitSizeCategory);
	Speed_ = InUnitDefinition->UnitSpeed;
	MaxAP_ = InUnitDefinition->UnitMaxAP;
	CurrentAP_ = MaxAP_;
}

FEssentialInfoForUnitActor UUnitBase::GetUnitActorEssentialInfo() const
{
	FEssentialInfoForUnitActor UnitActorEssentialInfo;
	UnitActorEssentialInfo.UnitCombatID = UnitCombatID_;
	UnitActorEssentialInfo.UnitSize = UnitSize_;
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
		UnitSize_.GetUnitSize(), *UnitSize_.GetUnitTileLength().ToString());
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
		UnitSize_.GetUnitSize(), UnitSize_.GetUnitTileLength().X, UnitSize_.GetUnitTileLength().Y,
		UnitPosition.GetUnitWorldPosition().X, UnitPosition.GetUnitWorldPosition().Y,
		UnitPosition.GetUnitTilePosition().X, UnitPosition.GetUnitTilePosition().Y,
		UnitRotation.GetUnitFRotation()
	);
}

bool UUnitBase::TryParseUnitSizeEnum(const FString& InString, EUnitSizeCategory& OutValue)
{
	UEnum* Enum = StaticEnum<EUnitSizeCategory>();
	if (!Enum)
	{
		return false;
	}
	
	FString Normalized = InString.TrimStartAndEnd();
	
	int32 ScopeIdx = Normalized.Find(TEXT("::"));
	if (ScopeIdx != INDEX_NONE)
	{
		Normalized = Normalized.RightChop(ScopeIdx + 2);
	}
	
	const int64 Value = Enum->GetValueByNameString(Normalized);
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutValue = static_cast<EUnitSizeCategory>(Value);
	return true;
}

