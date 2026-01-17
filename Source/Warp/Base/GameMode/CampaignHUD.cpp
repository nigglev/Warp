// Fill out your copyright notice in the Description page of Project Settings.


#include "CampaignHUD.h"

#include "Blueprint/UserWidget.h"

void ACampaignHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACampaignHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (!MainWidgetClass_ || MainWidget_)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	MainWidget_ = CreateWidget<UUserWidget>(PC, MainWidgetClass_);
	if (MainWidget_)
	{
		MainWidget_->AddToViewport(0); // ZOrder при желании
	}
	
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = false;

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(MainWidget_->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // или LockAlways/LockInFullscreen
	PC->SetInputMode(Mode);
}
