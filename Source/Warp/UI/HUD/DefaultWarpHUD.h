// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexPathfainer.h"
#include "GameFramework/HUD.h"
#include "Warp/Utils/RepAxialCoord.h"
#include "DefaultWarpHUD.generated.h"

class ABaseUnitActor;
class UEndTurnWidget;
class AWarpGameState;
class UTurnBasedSystemManager;
class UUnitSpawnWidget;
class STurnOrderWidget;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultWarpHUD : public AHUD
{
	GENERATED_BODY()

	
public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void ShowDebugHUD();

	virtual void DrawHUD() override;

protected:
	AWarpGameState* GetGameState() const;
	
	void OnMatchStateChanged(FName InMatchState);
	void OnUnitSelected(ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit);
	
	bool bShowDebugHUD_ = false;
	
	TArray<HexMath::FPathNode> InfluenceZone_;
};
