// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultWarpHUD.h"

#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"

DEFINE_LOG_CATEGORY_STATIC(ADefaultWarpHUDLog, Log, All);

AWarpGameState* ADefaultWarpHUD::GetGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWarpGameState>() : nullptr;
}

void ADefaultWarpHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	MG_COND_ERROR(ADefaultWarpHUDLog, PC == nullptr, TEXT("Player Controller Invalid"));
	if (PC != nullptr)
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}
	
	AWarpGameState* GS = GetGameState();
	MG_COND_ERROR(ADefaultWarpHUDLog, GS == nullptr, TEXT("Warp Game State Invalid"));
	if (GS != nullptr)
	{
		GS->OnUnitSelected.AddUObject(this, &ADefaultWarpHUD::OnUnitSelected);
	}
}

void ADefaultWarpHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ADefaultWarpHUD::ShowDebugHUD()
{
	bShowDebugHUD_ = !bShowDebugHUD_;
}

void ADefaultWarpHUD::DrawHUD()
{
	Super::DrawHUD();
	
	if (bShowDebugHUD_)
	{
		UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
		APlayerController* PC = GetOwningPlayerController();
		
		for (HexMath::FAxialCoord Hex : InfluenceZone_)
		{
			TOptional<FVector> PosOpt = UHexGridWorldSubsystem::AxialCellToWorldCoord(Hex, 0);
			if (PosOpt.IsSet())
			{
				FVector2D ScreenPos;
				const bool bOnScreen = PC->ProjectWorldLocationToScreen(PosOpt.GetValue(), ScreenPos, /*bPlayerViewportRelative*/ true);

				if (bOnScreen)
				{
					float StrWidth;
					float StrHeight;
					GetTextSize(Hex.ToString(), StrWidth, StrHeight);
					DrawText(Hex.ToString(), FLinearColor::Green, ScreenPos.X - StrWidth / 2, ScreenPos.Y - StrHeight / 2);//, GEngine->GetMediumFont(), 1.0f, false)
				}
			}
		}		
	}
}

void ADefaultWarpHUD::OnUnitSelected(ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit)
{
	RETURN_ON_FAIL(ADefaultWarpHUDLog, InNewActiveUnit);
	
	UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
	
	if (InPrevActiveUnit != nullptr)
	{
		MG_LOG(ADefaultWarpHUDLog, TEXT("%s[%s] -> %s[%s]"),
		   *GetNameSafe(InPrevActiveUnit), *InPrevActiveUnit->GetAxialCoord().ToString(),
		   *GetNameSafe(InNewActiveUnit), *InNewActiveUnit->GetAxialCoord().ToString());
	
		uint32 PrevUnitId = InPrevActiveUnit->GetUniqueID();
		GridWorldSubsystem->RemoveInfluence(PrevUnitId);
	}
	else
	{
		MG_LOG(ADefaultWarpHUDLog, TEXT("%s[%s]"),
		   *GetNameSafe(InNewActiveUnit), *InNewActiveUnit->GetAxialCoord().ToString());
	}
	
	uint32 UnitId = InNewActiveUnit->GetUniqueID();
	HexMath::FAxialCoord AC = InNewActiveUnit->GetAxialCoord();
	FAxialAngle AA = InNewActiveUnit->GetAxialAngle();
	
	GridWorldSubsystem->SelectInfluence(UnitId, AC, AA.R, 5, &InfluenceZone_);
}

