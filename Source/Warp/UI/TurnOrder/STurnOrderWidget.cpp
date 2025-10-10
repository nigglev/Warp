// Fill out your copyright notice in the Description page of Project Settings.


#include "STurnOrderWidget.h"


void STurnOrderWidget::Construct(const FArguments& InArgs)
{
	Font        = InArgs._Font;
	CurrentColor= InArgs._CurrentColor;
	OtherColor  = InArgs._OtherColor;
	BarColor    = InArgs._BarColor;
	Padding     = InArgs._Padding;
	Gap         = InArgs._Gap;
	Outline     = InArgs._Outline;

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(BarColor)
		.Padding(Outline)
		[
			SAssignNew(Row, SHorizontalBox)
		]
	];

	RebuildRow();
}

void STurnOrderWidget::SetOrder(const TArray<uint32>& InOrder, uint32 InCurrentId)
{
	Order     = InOrder;
	CurrentId = InCurrentId;
	RebuildRow();
}

void STurnOrderWidget::RebuildRow()
{
	if (!Row.IsValid()) return;

	Row->ClearChildren();

	for (int32 i = 0; i < Order.Num(); ++i)
	{
		const uint32 Id = Order[i];
		const bool bIsCurrent = (Id == CurrentId);

		TSharedRef<SBorder> Pill = SNew(SBorder)
			.BorderBackgroundColor(bIsCurrent
				? FLinearColor(CurrentColor.R, CurrentColor.G, CurrentColor.B, 0.9f)
				: FLinearColor(0.f, 0.f, 0.f, 0.4f))
			.Padding(Padding)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber((int32)Id).ToUpper())  // simple label
				.ColorAndOpacity(bIsCurrent ? FSlateColor(FLinearColor::Black) : FSlateColor(OtherColor))
				.Font(Font)
				.Justification(ETextJustify::Center)
			];

		Row->AddSlot()
			.AutoWidth()
			.Padding(FMargin(i == 0 ? 0.f : Gap, 0.f, 0.f, 0.f))
			[
				Pill
			];
	}
}