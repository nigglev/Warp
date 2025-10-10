// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameMode.generated.h"

class ADefaultPlayerController;
class ACombatMapManager;
class AWarpGameState;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADefaultGameMode();
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
protected:
	AWarpGameState* GetWarpGameState() const;
	void CreateUnitOnLogin(uint8 TeamID) const;
};



