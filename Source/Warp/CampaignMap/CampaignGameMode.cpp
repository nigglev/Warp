// Fill out your copyright notice in the Description page of Project Settings.


#include "CampaignGameMode.h"

#include "Warp/Base/GameMode/CampaignHUD.h"

DEFINE_LOG_CATEGORY_STATIC(ACampaignGameModeLog, Log, All);

ACampaignGameMode::ACampaignGameMode()
{
	HUDClass = ACampaignHUD::StaticClass();
}