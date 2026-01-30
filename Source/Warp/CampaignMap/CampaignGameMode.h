// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapNodeWidget.h"
#include "GameFramework/GameModeBase.h"
#include "CampaignGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API ACampaignGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ACampaignGameMode();
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	void OnCapture(FNodePosition InNodePosition, EMapNodeType InMapNode);
	
	FNodePosition GetNodePosition() const { return  NodePosition_; }
	
protected:
	UPROPERTY(EditAnywhere, Category="Map") TMap<uint8, FName> LayerToBattleMap;
	UPROPERTY(EditAnywhere, Category="Map") FName DefaultBattleMap;
	
	FNodePosition NodePosition_ = FNodePosition(0, 0);
};
