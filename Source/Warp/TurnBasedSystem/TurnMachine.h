// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TurnMachine.generated.h"

class ADefaultPlayerController;
class ABaseUnitActor;
class AWarpGameState;
/**
 * 
 */


UCLASS()
class WARP_API UTurnMachine : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	void Initialize(AWarpGameState* InGameState);
	void Start();

	void RefreshUnitsFromGameState();
	void OnTurnStateReplicated(); // optional

	// Called on SERVER from PlayerController RPC
	bool ServerRequestMove(const FVector& Target);

	bool CanAcceptMove() const;

	// UObject world
	virtual UWorld* GetWorld() const override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

protected:
	void ServerAdvanceTurn();
	ABaseUnitActor* GetServerActiveUnit() const;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<AWarpGameState> GameState_;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ABaseUnitActor>> Units_;

	bool bRunning_ = false;
};
