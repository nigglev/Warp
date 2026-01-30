// Fill out your copyright notice in the Description page of Project Settings.


#include "CampaignGameMode.h"
#include "CampaignHUD.h"
#include "MGLogs.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(ACampaignGameModeLog, Log, All);

ACampaignGameMode::ACampaignGameMode()
{
	HUDClass = ACampaignHUD::StaticClass();
}

void ACampaignGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	FString NodePositionStr = UGameplayStatics::ParseOption(Options, TEXT("NodePosition"));
	if (!NodePositionStr.IsEmpty())
	{
		bool bNodePositionParsed = NodePosition_.InitFromString(NodePositionStr);
		MG_COND_ERROR_SHORT(ACampaignGameModeLog, !bNodePositionParsed);
		MG_LOG(ACampaignGameModeLog, TEXT("NodePosition: %s"), *NodePosition_.ToString());
	}
}

void ACampaignGameMode::OnCapture(FNodePosition InNodePosition, EMapNodeType InMapNode)
{
	RETURN_ON_FAIL(ACampaignGameModeLog, InMapNode != EMapNodeType::Undefined);
	
	FName BattleMap = LayerToBattleMap.FindOrAdd(InNodePosition.X, DefaultBattleMap);
	
	//TEXT("Seed=123?Difficulty=Hard?From=MainMenu")
	
	FString MapType = StaticEnum<EMapNodeType>()->GetNameStringByValue(static_cast<int64>(InMapNode));
	
	UGameplayStatics::OpenLevel(this, BattleMap, true, 
		 FString::Printf(TEXT("MapType=%s?NodePosition=%s"), *MapType, *InNodePosition.ToString()));
}
