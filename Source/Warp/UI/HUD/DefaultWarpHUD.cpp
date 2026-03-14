// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultWarpHUD.h"

#include "MGLogs.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameMode.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/HexMap/HexMapWS.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"
#include "Warp/Base/GameState/TurnMachine.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"

DEFINE_LOG_CATEGORY_STATIC(ADefaultWarpHUDLog, Log, All);

AWarpGameState* ADefaultWarpHUD::GetGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWarpGameState>() : nullptr;
}

void ADefaultWarpHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	AWarpGameState* GS = GetGameState();
	MG_COND_ERROR(ADefaultWarpHUDLog, GS == nullptr, TEXT("Warp Game State Invalid"));
	if (GS != nullptr)
	{
		GS->OnMatchStateChanged.AddUObject(this, &ADefaultWarpHUD::OnMatchStateChanged);
		GS->OnUnitSelected.AddUObject(this, &ADefaultWarpHUD::OnUnitSelected);
		GS->OnUnitStartMoving.AddUObject(this, &ADefaultWarpHUD::OnUnitStartMoving);
	}
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
	
	RETURN_ON_FAIL(ADefaultWarpHUDLog, MainWidgetClass_);
	RETURN_ON_FAIL(ADefaultWarpHUDLog, MainWidget_ == nullptr);
	MainWidget_ = CreateWidget<UCombatUIWidget>(PC, MainWidgetClass_);
	if (MainWidget_)
	{
		MainWidget_->AddToViewport(0);
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
		APlayerController* PC = GetOwningPlayerController();
		
		for (HexMath::FPathNode Hex : InfluenceZone_)
		{
			TOptional<FVector> PosOpt = UHexMapWS::AxialCellToWorldCoord(Hex.Coord, 0);
			if (PosOpt.IsSet())
			{
				FVector2D ScreenPos;
				const bool bOnScreen = PC->ProjectWorldLocationToScreen(PosOpt.GetValue(), ScreenPos, /*bPlayerViewportRelative*/ true);

				if (bOnScreen)
				{
					float StrWidth;
					float StrHeight;
					GetTextSize(Hex.ToDebugScreenString(), StrWidth, StrHeight);
					DrawText(Hex.ToDebugScreenString(), FLinearColor::Green, ScreenPos.X - StrWidth / 2, ScreenPos.Y - StrHeight / 2);//, GEngine->GetMediumFont(), 1.0f, false)
				}
			}
		}		
	}
}

void ADefaultWarpHUD::OnMatchStateChanged(FName InMatchState)
{
	MG_LOG(ADefaultWarpHUDLog, TEXT("%s"), *InMatchState.ToString());
	
	if (InMatchState != MatchState::InProgress)
	{
		return;
	}
	
	AWarpGameState* GS = GetGameState();
	RETURN_ON_FAIL(ADefaultWarpHUDLog, GS);
	
	UTurnMachine* TM = GS->GetTurnMachine();
	RETURN_ON_FAIL(ADefaultWarpHUDLog, TM);
	
	ABaseUnitActor* Unit = TM->GetActiveUnit();
	OnUnitSelected(Unit, nullptr);
}

void ADefaultWarpHUD::OnUnitSelected(ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit)
{
	RETURN_ON_FAIL(ADefaultWarpHUDLog, InNewActiveUnit);
	
	AWarpGameState* GS = GetGameState();
	RETURN_ON_FAIL(ADefaultWarpHUDLog, GS);
	
	if (GS->GetMatchState() != MatchState::InProgress)
	{
		return;
	}
		
	if (InPrevActiveUnit != nullptr)
	{
		MG_LOG(ADefaultWarpHUDLog, TEXT("%s[%s] -> %s[%s]"),
		   *InPrevActiveUnit->GetDebugName(), *InPrevActiveUnit->GetAxialPosition().ToString(),
		   *InNewActiveUnit->GetDebugName(), *InNewActiveUnit->GetAxialPosition().ToString());
	}
	else
	{
		MG_LOG(ADefaultWarpHUDLog, TEXT("%s[%s]"),
		   *InNewActiveUnit->GetDebugName(), *InNewActiveUnit->GetAxialPosition().ToString());
	}
	
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	
	if (InfluenceZoneId_.IsSet())
		HexMapWS->RemoveInfluence(InfluenceZoneId_.GetValue());
	
	InfluenceZoneId_ = InNewActiveUnit->GetUniqueID();
	HexMath::FAxialCoord AC = InNewActiveUnit->GetAxialPosition();
	FAxialAngle AA = InNewActiveUnit->GetAxialRotation();
	
	InfluenceZone_.Empty();
	HexMapWS->SelectInfluence(InfluenceZoneId_.GetValue(), AC, AA.R, InNewActiveUnit->GetCurrentMoveParams(), &InfluenceZone_);
	
	RETURN_ON_FAIL(ADefaultWarpHUDLog, MainWidget_);
	MainWidget_->OnUnitSelected(InNewActiveUnit, InPrevActiveUnit);
}

void ADefaultWarpHUD::OnUnitStartMoving(ABaseUnitActor* InNewActiveUnit)
{
	UHexMapWS* HexMapWS = UHexMapWS::Get(this);
	
	if (InfluenceZoneId_.IsSet())
		HexMapWS->RemoveInfluence(InfluenceZoneId_.GetValue());
	
	RETURN_ON_FAIL(ADefaultWarpHUDLog, MainWidget_);
	MainWidget_->OnUnitStartMoving(InNewActiveUnit);
}

