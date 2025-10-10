// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DefaultWarpHUD.generated.h"

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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	APlayerController* Init() const;
	void CreateSpawnWidgets(APlayerController* InPC);
	void BuildTurnOrderOverlay();
	AWarpGameState* GetGameState() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
	UFUNCTION()
	void UI_SpawnSmall();
	UFUNCTION()
	void UI_SpawnMedium();
	UFUNCTION()
	void UI_SpawnLarge();
	UFUNCTION()
	void HandleTurnOrderUpdated(const TArray<uint32>& Order, uint32 CurrentId);
	void PushInitialTurnOrder();

	UPROPERTY()
	UUnitSpawnWidget* SpawnWidget = nullptr;
	
	TSharedPtr<SOverlay> RootOverlay;
	TSharedPtr<STurnOrderWidget> TurnOrderSlate;

	FDelegateHandle TurnOrderUpdatedHandle;
};
