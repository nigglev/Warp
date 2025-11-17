// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultWarpHUD.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Blueprint/UserWidget.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UI/CombatUI/StartBattleWidget.h"
DEFINE_LOG_CATEGORY_STATIC(ADefaultWarpHUDLog, Log, All);

void ADefaultWarpHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Init();

	if (StartBattleWidgetClass)
	{
		StartBattleWidget = CreateWidget<UStartBattleWidget>(PC, StartBattleWidgetClass);
		if (StartBattleWidget)
		{
			StartBattleWidget->AddToViewport();
		}
	}
}

void ADefaultWarpHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);
}

APlayerController* ADefaultWarpHUD::Init() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		MG_COND_ERROR(ADefaultWarpHUDLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMap), TEXT("Player Controller Invalid"));
		return nullptr;
	}
	
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	
	return PC;
}


AWarpGameState* ADefaultWarpHUD::GetGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWarpGameState>() : nullptr;
}

UTurnBasedSystemManager* ADefaultWarpHUD::GetTurnBasedSystemManager() const
{
	return GetGameState() ? GetGameState()->GetTurnBasedSystemManager() : nullptr;
}
