// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

#include "IContentBrowserSingleton.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/GameInstanceSubsystem/UnitDataSubsystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UnitStaticData/PlayFabUnitTypes.h"

DEFINE_LOG_CATEGORY_STATIC(ADefaultPlayerControllerLog, Log, All);

ADefaultPlayerController::ADefaultPlayerController()
{
	bReplicates = true;
	bShowMouseCursor = true;
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetupEnhancedInput();
	CreateCombatMapManager();
}

void ADefaultPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateTileHovering();
	UpdateUnitGhostPosition();
}

void ADefaultPlayerController::SetupEnhancedInput() const
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADefaultPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(RotateUnitGhostAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnRotateUnitGhostAction);

		EIC->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraMove);
		EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraZoom);
		
		EIC->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraRotate);
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Started,   this, &ADefaultPlayerController::OnRotateCameraPressed);
		
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnRotateCameraReleased);
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Canceled,  this, &ADefaultPlayerController::OnRotateCameraReleased);

		EIC->BindAction(MoveUnitAction, ETriggerEvent::Started,   this, &ADefaultPlayerController::OnMoveUnitAction);
	}
}


void ADefaultPlayerController::CreateCombatMapManager()
{
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, IsLocalController());
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, CombatMapManagerClass);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CombatMapManager = GetWorld()->SpawnActor<ACombatMapManager>(
		CombatMapManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Spawned local CombatMapManager [%s]"), *GetNameSafe(CombatMapManager));
	
	TryInitCombatMapManager();
	if (!InitMapTimerHandle.IsValid())
	{
		GetWorldTimerManager().SetTimer(InitMapTimerHandle, this,
			&ADefaultPlayerController::TryInitCombatMapManager, 0.1f, true);
	}
	
}

void ADefaultPlayerController::TryInitCombatMapManager()
{
	if (!CombatMapManager) return;

	if (AWarpGameState* GS = GetGameState())
	{
		const uint32 Grid = GS->GetMapGridSize();
		const uint32 Tile = GS->GetMapTileSize();

		if (Grid > 0 && Tile > 0)
		{
			CombatMapManager->Init(Grid, Tile);
			CombatMapManager->GenerateGrid();

			if (InitMapTimerHandle.IsValid())
			{
				GetWorldTimerManager().ClearTimer(InitMapTimerHandle);
			}

			MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
				TEXT("CombatMapManager initialized from GameState (Grid=%d, Tile=%d)"), Grid, Tile);
		}
	}
}

void ADefaultPlayerController::ServerStartCombat_Implementation()
{
	if (UTurnBasedSystemManager* TBSM = GetGameState()->GetTurnBasedSystemManager())
	{
		TBSM->StartCombat();
		
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Start combat"));
	}
}

void ADefaultPlayerController::UpdateTileHovering()
{
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, CombatMapManager == nullptr);

	int32 TileIndex;
	PrevHoveredTile = HoveredTile;
	if (!GetHoveredTileIndexAndCoordinates(TileIndex, HoveredTile))
		return;
	if (TileIndex < 0 || HoveredTile.X < 0 || HoveredTile.Y < 0)
		return;
	if (HoveredTile == PrevHoveredTile)
		return;
	CombatMapManager->UpdateHoveredTile(TileIndex);
}

void ADefaultPlayerController::UpdateUnitGhostPosition() const
{
	if ((!bPlacingUnit_ && !bMovingUnit_) || !IsValid(GhostActor_))
		return;
	if (HoveredTile == PrevHoveredTile)
		return;
	
	const FVector WorldCenter = CombatMapManager->GridToLevelPosition(HoveredTile);
	TArray<FIntPoint> OutBlockers;
	if (CombatMapManager->IsPositionAvailable(GhostActor_->GetUnitActorSize(), GhostActor_->GetUnitActorRotation(), HoveredTile, OutBlockers))
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("GOOD POSITION"));
	}
	CombatMapManager->UpdateVisualForBlockers(OutBlockers);
	GhostActor_->UpdatePosition(WorldCenter);
}

void ADefaultPlayerController::OnRotateUnitGhostAction(const FInputActionValue& Value)
{
	if (!IsValid(GhostActor_))
		return;
	const float Delta = Value.Get<float>();
	if (Delta > 0)
		GhostActor_->Rotate(true);
	if (Delta < 0)
		GhostActor_->Rotate(false);
}

void ADefaultPlayerController::OnMoveUnitAction()
{
	if (bPlacingUnit_)
		return;
	
	bMovingUnit_ = !bMovingUnit_;
	if (bMovingUnit_)
	{
		SelectedUnitID = GetMouseoverUnitID();
		if (SelectedUnitID == 0)
			return;
		GhostActor_ = CombatMapManager->SpawnUnitActorGhost(SelectedUnitID);
	}
	else
	{
		if (SelectedUnitID == 0)
			return;
		MoveUnitServerAuthoritative(SelectedUnitID, HoveredTile);
		CombatMapManager->DestroyGhostActor(GhostActor_);
		SelectedUnitID = 0;
	}
	
}

void ADefaultPlayerController::OnCameraMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->MoveXY(Axis, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f);
	}
}

void ADefaultPlayerController::OnCameraRotate(const FInputActionValue& Value)
{
	if (!bRotateCamera) return;

	const float MouseX = Value.Get<float>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->AddRotation(MouseX * MouseYawScaleDegPerUnit);
	}
}

void ADefaultPlayerController::OnRotateCameraPressed(const FInputActionValue& Value)
{
	bRotateCamera = true;
}

void ADefaultPlayerController::OnRotateCameraReleased(const FInputActionValue& Value)
{
	bRotateCamera = false;
}

void ADefaultPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->AddZoom(Axis);
	}
}

bool ADefaultPlayerController::MoveUnitServerAuthoritative(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition)
{
	if (!CombatMapManager) return false;
	TArray<FIntPoint> Blockers;

	if (!CombatMapManager->IsPositionForUnitAvailable(InUnitToMoveID, InGridPosition, Blockers))
		return false;

	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("Requesting to place unit %d at a pos %s"),
	InUnitToMoveID, *InGridPosition.ToString());

	CombatMapManager->MoveUnitTo(InUnitToMoveID, InGridPosition);
	
	ServerRequestMoveUnit(InUnitToMoveID, InGridPosition);
	
	return true;
}

void ADefaultPlayerController::ServerRequestMoveUnit_Implementation(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition)
{
	AWarpGameState* GS = GetGameState();
	if (!IsValid(GS))
	{
		ClientPlacementResult(false);
		return;
	}
	
	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("Requesting to move unit ID = %d at a pos %s"), InUnitToMoveID, *InGridPosition.ToString());
	
	bool bSuccess = GS->MoveUnitTo(InUnitToMoveID, InGridPosition);
	ClientPlacementResult(bSuccess);
}

void ADefaultPlayerController::ClientPlacementResult_Implementation(bool bSuccess)
{
	if (bSuccess)
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Unit placed successfully"));
	}
	else
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Failed to place unit"));
	}
}

uint32 ADefaultPlayerController::GetMouseoverUnitID() const
{
	FHitResult Hit;
	if (GetHitResultUnderCursorByChannel(TraceTypeQuery1, true, Hit))
	{
		ABaseUnitActor* A = Cast<ABaseUnitActor>(Hit.GetActor());
		if (!A)
		{
			MG_COND_WARNING(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("Failed to get actor on mouseover"));

			return 0;
		}

		uint32 AId = A->GetID();

		if (AId == 0)
		{
			MG_COND_WARNING(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("Failed to get actor ID on mouseover"));
		}
		
		return AId;
	}
	return 0;
}


bool ADefaultPlayerController::GetHoveredTileIndexAndCoordinates(int32& OutInstanceIndex, FIntVector2& OutCoord) const
{
	int32 HitIdx;
	if (GetHoveredTileIndex(HitIdx))
	{
		OutInstanceIndex = HitIdx;
		OutCoord = CombatMapManager->TileInstanceToGridPosition(HitIdx);
		return true;
	}
	return false;
}

bool ADefaultPlayerController::GetHoveredTileIndex(int32& OutInstanceIndex) const
{
	OutInstanceIndex = INDEX_NONE;

	FHitResult Hit;
	if (!GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
	{
		return false;
	}
	
	if (CombatMapManager && Hit.Component == CombatMapManager->GetTilesISMC())
	{
		int32 TileIndex;
		if (Hit.Item >= CombatMapManager->GetGridSquare())
			TileIndex = Hit.Item - CombatMapManager->GetGridSquare();
		else
		{
			TileIndex = Hit.Item;
		}
		OutInstanceIndex = TileIndex;
		return true;
	}

	return false;
}

AWarpGameState* ADefaultPlayerController::GetGameState() const
{
	if (AWarpGameState* GameState = GetWorld()->GetGameState<AWarpGameState>())
	{
		return GameState;
	}
	return nullptr;
}

UUnitDataSubsystem* ADefaultPlayerController::GetUnitDataSubsystem(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UUnitDataSubsystem>();
		}
	}
	return nullptr;
}
