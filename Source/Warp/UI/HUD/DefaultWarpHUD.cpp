// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultWarpHUD.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Blueprint/UserWidget.h"
#include "Warp/UnitSpawnWidget.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UI/TurnOrder/STurnOrderWidget.h"
DEFINE_LOG_CATEGORY_STATIC(ADefaultWarpHUDLog, Log, All);

void ADefaultWarpHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Init();
	CreateSpawnWidgets(PC);
	BuildTurnOrderOverlay();
	PushInitialTurnOrder();
}

void ADefaultWarpHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TurnOrderUpdatedHandle.IsValid())
	{
		GetTurnBasedSystemManager()->OnTurnOrderUpdated.Remove(TurnOrderUpdatedHandle);
		TurnOrderUpdatedHandle.Reset();
	}


	if (GEngine && GEngine->GameViewport && RootOverlay.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootOverlay.ToSharedRef());
		RootOverlay.Reset();
		TurnOrderSlate.Reset();
	}

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

void ADefaultWarpHUD::CreateSpawnWidgets(APlayerController* InPC)
{
	SpawnWidget = CreateWidget<UUnitSpawnWidget>(GetWorld(), UUnitSpawnWidget::StaticClass());
	if (SpawnWidget)
	{
		SpawnWidget->AddToViewport();
		SpawnWidget->SetOwningPlayer(InPC);
		SpawnWidget->OnSpawnSmall.AddDynamic(this, &ThisClass::UI_SpawnSmall);
		SpawnWidget->OnSpawnMedium.AddDynamic(this, &ThisClass::UI_SpawnMedium);
		SpawnWidget->OnSpawnLarge.AddDynamic(this, &ThisClass::UI_SpawnLarge);
	}
}

void ADefaultWarpHUD::BuildTurnOrderOverlay()
{
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(RootOverlay, SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 12.f, 0.f, 0.f))
		[
			SAssignNew(TurnOrderSlate, STurnOrderWidget)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
		];

		GEngine->GameViewport->AddViewportWidgetContent(RootOverlay.ToSharedRef(), /*ZOrder*/ 200);
	}
	
	UTurnBasedSystemManager* TB = GetTurnBasedSystemManager();
	if (!TB) return;
	TurnOrderUpdatedHandle = TB->OnTurnOrderUpdated.AddUObject(this, &ADefaultWarpHUD::HandleTurnOrderUpdated);
	
}


void ADefaultWarpHUD::HandleTurnOrderUpdated(const TArray<uint32>& Order, uint32 CurrentId)
{
	if (TurnOrderSlate.IsValid())
	{
		TurnOrderSlate->SetOrder(Order, CurrentId);
	}
}

void ADefaultWarpHUD::PushInitialTurnOrder()
{
	UTurnBasedSystemManager* TB = GetTurnBasedSystemManager();
	if (!TurnOrderSlate.IsValid()) return;
	if (!TB) return;
	
	const TArray<uint32>& Order = TB->GetTurnOrderUnitIds();
	if (Order.Num() <= 0)
	{
		MG_COND_LOG(ADefaultWarpHUDLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("No Turn Order to display"));
		return;
	}
		
	uint32 Current = 0;
	if (TB->GetCurrentTurnUnit() != nullptr)
		Current = TB->GetCurrentTurnUnit()->GetUnitID();
	if (Order.Num() > 0)
	{
		TurnOrderSlate->SetOrder(Order, Current);
	}

}

void ADefaultWarpHUD::UI_SpawnSmall()
{
	MG_COND_LOG(ADefaultWarpHUDLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("[HUD] Small unit button clicked"));

	if (auto* PC = Cast<ADefaultPlayerController>(GetOwningPlayerController()))
	{
		FUnitSize Size;
		Size.SetUnitSize(EUnitSizeCategory::Small);
		PC->CancelPlacementMode();
		PC->EnterPlacementMode(Size);
	}
}

void ADefaultWarpHUD::UI_SpawnMedium()
{
	MG_COND_LOG(ADefaultWarpHUDLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("[HUD] Medium unit button clicked"));
	
	if (auto* PC = Cast<ADefaultPlayerController>(GetOwningPlayerController()))
	{
		FUnitSize Size;
		Size.SetUnitSize(EUnitSizeCategory::Medium);
		PC->CancelPlacementMode();
		PC->EnterPlacementMode(Size);
	}
}

void ADefaultWarpHUD::UI_SpawnLarge()
{
	MG_COND_LOG(ADefaultWarpHUDLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("[HUD] Large unit button clicked"));

	if (auto* PC = Cast<ADefaultPlayerController>(GetOwningPlayerController()))
	{
		FUnitSize Size;
		Size.SetUnitSize(EUnitSizeCategory::Big);
		PC->CancelPlacementMode();
		PC->EnterPlacementMode(Size);
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
