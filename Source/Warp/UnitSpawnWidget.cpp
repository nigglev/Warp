// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitSpawnWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"

DEFINE_LOG_CATEGORY_STATIC(UUnitSpawnWidgetLog, Log, All);

void UUnitSpawnWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BtnSmall)
		{ BtnSmall->OnClicked.AddDynamic(this, &ThisClass::HandleSmallClicked); }
	if (BtnMedium)
		{ BtnMedium->OnClicked.AddDynamic(this, &ThisClass::HandleMediumClicked); }
	if (BtnLarge)
		{ BtnLarge->OnClicked.AddDynamic(this, &ThisClass::HandleLargeClicked); }
}

TSharedRef<SWidget> UUnitSpawnWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;
	
	ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
	UCanvasPanelSlot* RowSlot = RootCanvas->AddChildToCanvas(ButtonRow);
	RowSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
	RowSlot->SetAlignment(FVector2D(0.f, 1.f));
	RowSlot->SetPosition(FVector2D(16.f, -72.f));
	RowSlot->SetSize(FVector2D(420.f, 48.f));

	BtnSmall = MakeButton(TEXT("Small"));
	BtnMedium = MakeButton(TEXT("Medium"));
	BtnLarge = MakeButton(TEXT("Large"));

	if (UHorizontalBoxSlot* S = ButtonRow->AddChildToHorizontalBox(BtnSmall))
		S->SetPadding(FMargin(4.f, 0.f));
	if (UHorizontalBoxSlot* S = ButtonRow->AddChildToHorizontalBox(BtnMedium))
		S->SetPadding(FMargin(4.f, 0.f));
	if (UHorizontalBoxSlot* S = ButtonRow->AddChildToHorizontalBox(BtnLarge))
		S->SetPadding(FMargin(4.f, 0.f));
	
	return Super::RebuildWidget();
}


UButton* UUnitSpawnWidget::MakeButton(const FString& Label) const
{
	UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Txt->SetText(FText::FromString(Label));
	Btn->AddChild(Txt);
	
	if (UHorizontalBoxSlot* AsSlot = Cast<UHorizontalBoxSlot>(Btn->Slot))
	{
		AsSlot->SetPadding(FMargin(4.f, 0.f));
	}

	return Btn;
}

void UUnitSpawnWidget::HandleSmallClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("[HUD] Spawn SMALL requested"));
	OnSpawnSmall.Broadcast();
}

void UUnitSpawnWidget::HandleMediumClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("[HUD] Spawn MEDIUM requested"));
	OnSpawnMedium.Broadcast();
}

void UUnitSpawnWidget::HandleLargeClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("[HUD] Spawn LARGE requested"));
	OnSpawnLarge.Broadcast();
}
