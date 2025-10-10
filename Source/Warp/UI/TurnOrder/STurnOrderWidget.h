// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STurnOrderWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STurnOrderWidget)
		: _Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
		, _CurrentColor(FLinearColor(0.95f, 0.75f, 0.2f, 1.f))   // gold
		, _OtherColor(FLinearColor::White)
		, _BarColor(FLinearColor(0.f, 0.f, 0.f, 0.35f))
		, _Padding(FMargin(8.f, 4.f))
		, _Gap(10.f)
		, _Outline(8.f)
	{}
	SLATE_ARGUMENT(FSlateFontInfo,  Font)
	SLATE_ARGUMENT(FLinearColor,    CurrentColor)
	SLATE_ARGUMENT(FLinearColor,    OtherColor)
	SLATE_ARGUMENT(FLinearColor,    BarColor)
	SLATE_ARGUMENT(FMargin,         Padding)
	SLATE_ARGUMENT(float,           Gap)
	SLATE_ARGUMENT(float,           Outline)
	SLATE_END_ARGS()

void Construct(const FArguments& InArgs);
	
	void SetOrder(const TArray<uint32>& InOrder, uint32 InCurrentId);

private:
	void RebuildRow();
	
	TArray<uint32> Order;
	uint32 CurrentId = 0;

	TSharedPtr<SHorizontalBox> Row;

	FSlateFontInfo Font;
	FLinearColor   CurrentColor;
	FLinearColor   OtherColor;
	FLinearColor   BarColor;
	FMargin        Padding;
	float          Gap = 10.f;
	float          Outline = 8.f;
};