// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpCheatManager.h"

#include "MGLogs.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(WarpCheatManagerLog, Log, All);

void UWarpCheatManager::SaveDescriptionToPlayFab(const FString& InDescriptionName)
{
	auto CS = UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(WarpCheatManagerLog, CS != nullptr);
	
	CS->SaveDescriptionToPlayFab(FName(*InDescriptionName));	
}


