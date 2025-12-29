// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "WarpCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UWarpCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
	UFUNCTION(Exec)
	void SaveDescriptionToPlayFab(const FString& InDescriptionName);
	UFUNCTION(Exec)
	void UpdateContent();
};
