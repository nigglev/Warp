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

void ADefaultWarpHUD::SetupWidgets(APlayerController* InPC)
{
	if (CombatUIWidgetClass_)
	{
		CombatUIWidget_ = CreateWidget<UCombatUIWidget>(InPC, CombatUIWidgetClass_);
		if (CombatUIWidget_)
		{
			CombatUIWidget_->AddToViewport();
		}
	}

	if (TurnOrderWidgetClass_)
	{
		TurnOrderWidget_ = CreateWidget<UTurnOrderWidget>(InPC, TurnOrderWidgetClass_);
		if (TurnOrderWidget_)
		{
			TurnOrderWidget_->Init(this);
			TurnOrderWidget_->AddToViewport();
		}
	}
}

void ADefaultWarpHUD::SetupTBSMEvents()
{
	GetTurnBasedSystemManager()->OnTurnOrderUpdated.AddUObject(
			this, &ADefaultWarpHUD::HandleTurnOrderUpdated);

	GetTurnBasedSystemManager()->OnActiveUnitChanged.AddUObject(
		this, &ADefaultWarpHUD::HandleActiveUnitChanged);
}

void ADefaultWarpHUD::HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitCombatIds, uint32 InCurrentTurnUnitCombatId)
{
	TurnOrderUnitCombatIds_ = InTurnOrderUnitCombatIds;
	CurrentTurnUnitCombatId_ = InCurrentTurnUnitCombatId;

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->RebuildFromHUD();
	}
}

void ADefaultWarpHUD::HandleActiveUnitChanged(uint32 InCurrentTurnUnitCombatId)
{
	CurrentTurnUnitCombatId_ = InCurrentTurnUnitCombatId;

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->UpdateCurrentFromHUD();
	}
}


void ADefaultWarpHUD::GetTurnOrderInfo(TArray<uint32>& OutUnitCombatIds, uint32& OutCurrentUnitCombatId) const
{
	OutUnitCombatIds = TurnOrderUnitCombatIds_;
	OutCurrentUnitCombatId = CurrentTurnUnitCombatId_;
}

void ADefaultWarpHUD::GetTurnOrderUnitInfo(uint32 InUnitCombatId, FTurnOrderUnitInfo& OutInfo) const
{
	RETURN_ON_FAIL(ADefaultWarpHUDLog, GetTurnBasedSystemManager());
	
	OutInfo.UnitTypeName_ = GetGameState()->GetUnitByID(InUnitCombatId)->GetUnitTypeName();
	EUnitAffiliation Affiliation = GetGameState()->GetUnitByID(InUnitCombatId)->GetUnitAffiliation();
	if (Affiliation == EUnitAffiliation::Ally || Affiliation == EUnitAffiliation::Player)
		OutInfo.bIsAlly_ = true;
	if (Affiliation == EUnitAffiliation::Enemy)
		OutInfo.bIsAlly_ = false;
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
	return CombatUIWidget_;
}

