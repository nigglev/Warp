#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UnitStandardAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class WARP_API UUnitStandardAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category="Ship|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UUnitStandardAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category="Ship|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UUnitStandardAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMovementPoints, Category="Ship|Attributes")
	FGameplayAttributeData MaxMovementPoints;
	ATTRIBUTE_ACCESSORS(UUnitStandardAttributeSet, MaxMovementPoints)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementPoints, Category="Ship|Attributes")
	FGameplayAttributeData MovementPoints;
	ATTRIBUTE_ACCESSORS(UUnitStandardAttributeSet, MovementPoints)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MovementPoints(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMovementPoints(const FGameplayAttributeData& OldValue);	
};
