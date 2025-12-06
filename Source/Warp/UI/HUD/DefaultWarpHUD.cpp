// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultWarpHUD.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Blueprint/UserWidget.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"
#include "Warp/UI/UnitOrderUI/TurnOrderUnitInfo.h"
#include "Warp/UI/UnitOrderUI/TurnOrderWidget.h"
DEFINE_LOG_CATEGORY_STATIC(ADefaultWarpHUDLog, Log, All);

void ADefaultWarpHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Init();
	SetupTBSMEvents();
	SetupWidgets(PC);
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

void ADefaultWarpHUD::SetupTBSMEvents()
{
	GetTurnBasedSystemManager()->OnTurnOrderUpdated.AddUObject(
			this, &ADefaultWarpHUD::HandleTurnOrderUpdated);

	GetTurnBasedSystemManager()->OnActiveUnitChanged.AddUObject(
		this, &ADefaultWarpHUD::HandleActiveUnitChanged);
}

void ADefaultWarpHUD::SetupWidgets(APlayerController* InPC)
{
	if (CombatUIWidgetClass)
	{
		CombatUIWidget = CreateWidget<UCombatUIWidget>(InPC, CombatUIWidgetClass);
		if (CombatUIWidget)
		{
			CombatUIWidget->AddToViewport();
		}
	}

	if (TurnOrderWidgetClass)
	{
		TurnOrderWidget = CreateWidget<UTurnOrderWidget>(InPC, TurnOrderWidgetClass);
		if (TurnOrderWidget)
		{
			TurnOrderWidget->AddToViewport();
		}
	}
}

void ADefaultWarpHUD::GetTurnOrder(TArray<uint32>& OutUnitIds, uint32& OutCurrentUnitId) const
{
	OutUnitIds = CachedTurnOrderUnitIds_;
	OutCurrentUnitId = CachedCurrentTurnUnitId_;
}

bool ADefaultWarpHUD::GetTurnOrderUnitInfo(FTurnOrderUnitInfo& OutInfo) const
{
	RETURN_ON_FAIL_BOOL(ADefaultWarpHUDLog, GetTurnBasedSystemManager());

	OutInfo.UnitCombatId_ = CachedCurrentTurnUnitId_;
	OutInfo.UnitTypeName_ = GetGameState()->GetUnitByID(CachedCurrentTurnUnitId_)->GetUnitTypeName();
	//OutInfo.bIsAlly_ = GetGameState()->GetUnitByID(OutInfo.UnitCombatId_)->GetUnitAffiliation();

	return true;
}

void ADefaultWarpHUD::HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitIds, uint32 InCurrentTurnUnitId)
{
	CachedTurnOrderUnitIds_ = InTurnOrderUnitIds;
	CachedCurrentTurnUnitId_ = InCurrentTurnUnitId;

	if (TurnOrderWidget)
	{
		//TurnOrderWidget->RebuildFromHUD();
	}
}

void ADefaultWarpHUD::HandleActiveUnitChanged(uint32 InCurrentTurnUnitId)
{
	CachedCurrentTurnUnitId_ = InCurrentTurnUnitId;

	if (TurnOrderWidget)
	{
		//TurnOrderWidget->UpdateCurrentFromHUD();
	}
}

AWarpGameState* ADefaultWarpHUD::GetGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWarpGameState>() : nullptr;
}

UTurnBasedSystemManager* ADefaultWarpHUD::GetTurnBasedSystemManager() const
{
	return GetGameState() ? GetGameState()->GetTurnBasedSystemManager() : nullptr;
}

UCombatUIWidget* ADefaultWarpHUD::GetCombatUI() const
{
	return CombatUIWidget;
}

