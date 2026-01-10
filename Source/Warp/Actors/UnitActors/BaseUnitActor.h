// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCharacteristics/UnitSize.h"
#include "BaseUnitActor.generated.h"

UENUM()
enum class EUnitActorState : uint8
{
	None = 0,
	Playable = 1,
	Ghost = 2,
	MAX
};

UCLASS()
class WARP_API ABaseUnitActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseUnitActor();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	uint32 GetID() const {return UnitID;}
	void SetID(const uint32 InID) {UnitID = InID;}

	FVector GetUnitWorldPosition() const {return GetActorLocation();}
	void SetUnitWorldPosition(const FVector& InWorldPosition) {SetActorLocation(InWorldPosition);}

	FUnitSize GetUnitActorSize() const {return UnitActorSize;}
	void SetUnitActorSize(const FUnitSize InSize) {UnitActorSize = InSize;}
	
protected:

	UPROPERTY(Replicated)
	uint32 UnitID = 0;
	UPROPERTY(Replicated)
	FUnitSize UnitActorSize = FUnitSize::None();
};
