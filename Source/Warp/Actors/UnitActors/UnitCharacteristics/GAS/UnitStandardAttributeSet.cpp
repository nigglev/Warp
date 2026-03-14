// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitStandardAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UUnitStandardAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UUnitStandardAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UUnitStandardAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UUnitStandardAttributeSet, MaxMovementPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UUnitStandardAttributeSet, MovementPoints, COND_None, REPNOTIFY_Always);
}

void UUnitStandardAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMovementPointsAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMovementPoints());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetMaxMovementPointsAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UUnitStandardAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMovementPointsAttribute())
	{
		SetMovementPoints(FMath::Clamp(GetMovementPoints(), 0.0f, GetMaxMovementPoints()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxMovementPointsAttribute())
	{
		SetMovementPoints(FMath::Clamp(GetMovementPoints(), 0.0f, GetMaxMovementPoints()));
	}
}

void UUnitStandardAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUnitStandardAttributeSet, Health, OldValue);
}

void UUnitStandardAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUnitStandardAttributeSet, MaxHealth, OldValue);
}

void UUnitStandardAttributeSet::OnRep_MovementPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUnitStandardAttributeSet, MovementPoints, OldValue);
}

void UUnitStandardAttributeSet::OnRep_MaxMovementPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUnitStandardAttributeSet, MaxMovementPoints, OldValue);
}
