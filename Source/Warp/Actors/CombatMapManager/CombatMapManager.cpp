// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatMapManager.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/GameInstanceSubsystem/UnitDataSubsystem.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/Units/UnitBase.h"

DEFINE_LOG_CATEGORY_STATIC(ACombatMapManagerLog, Log, All);


// Sets default values
ACombatMapManager::ACombatMapManager()
{
	bReplicates = false;
	TilesISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Tiles"));
	RootComponent = TilesISMC;
	
	HoverISMC   = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Hover"));
	PreviewISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Preview"));
	BlockedISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Blocked"));
	MovementISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Movement"));
	
	HoverISMC->SetupAttachment(RootComponent);
	PreviewISMC->SetupAttachment(RootComponent);
	BlockedISMC->SetupAttachment(RootComponent);
	MovementISMC->SetupAttachment(RootComponent);
	
	HoverISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockedISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	HoverISMC->SetTranslucentSortPriority(5);
	BlockedISMC->SetTranslucentSortPriority(7);
	PreviewISMC->SetTranslucentSortPriority(15);
	MovementISMC->SetTranslucentSortPriority(10);
	
}

void ACombatMapManager::BeginPlay()
{
	Super::BeginPlay();
	OwnerPC = Cast<APlayerController>(GetOwner());
	SetupLayers();
	
	if (AWarpGameState* GS = GetGameState())
	{
		GS->OnUnitsReplicated.AddUObject(this, &ACombatMapManager::SpawnUnitActors);
		if (!IsInitialUnitsSpawned)
		{
			SpawnUnitActors(GS->GetActiveUnits());
		}
		MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
			TEXT("Subscribed to Unit replication"));
	}
}

void ACombatMapManager::SetupLayers() const
{
	if (TileMesh == nullptr)
	{
		UE_LOG(ACombatMapManagerLog, Warning, TEXT("TileMesh not assigned on %s"), *GetName());
	}
	TilesISMC->SetStaticMesh(TileMesh);
	HoverISMC->SetStaticMesh(TileMesh);
	PreviewISMC->SetStaticMesh(TileMesh);
	BlockedISMC->SetStaticMesh(TileMesh);
	MovementISMC->SetStaticMesh(TileMesh);
	
	if (BaseMaterial)    TilesISMC->SetMaterial(0, BaseMaterial);
	if (HoverMaterial)   HoverISMC->SetMaterial(0, HoverMaterial);
	if (PreviewMaterial) PreviewISMC->SetMaterial(0, PreviewMaterial);
	if (BlockedMaterial) BlockedISMC->SetMaterial(0, BlockedMaterial);
	if (MovementMaterial) MovementISMC->SetMaterial(0, MovementMaterial);

	TilesISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TilesISMC->SetCollisionResponseToAllChannels(ECR_Ignore);
	TilesISMC->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HoverISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockedISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	HoverISMC->ClearInstances();
	PreviewISMC->ClearInstances();
	BlockedISMC->ClearInstances();
	MovementISMC->ClearInstances();
}


void ACombatMapManager::Init(const uint32 InGridSize, const uint32 InTileSize)
{
	GridSize = InGridSize;
	TileSize = InTileSize * MetresToUUScale;
	GridOrigin = FVector::ZeroVector;
	MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
		TEXT("CombatMap Grid Manager Size = %d. CombatMap Tile Manager Size = %d"), GridSize, TileSize);
}

void ACombatMapManager::GenerateGrid()
{
	const FBoxSphereBounds MeshBounds = TilesISMC->GetStaticMesh()->GetBounds();
	const float MeshWidth = MeshBounds.BoxExtent.X * 2.f;
	const float UniformScale = TileSize / MeshWidth;

	for (uint32 X = 0; X < GridSize; ++X)
	{
		for (uint32 Y = 0; Y < GridSize; ++Y)
		{
			const FVector LocalPos(X * TileSize, Y * TileSize, 0.f);
			const FTransform LocalTM(FRotator::ZeroRotator, LocalPos, FVector(UniformScale));
			TilesTransformsLocal.Add(LocalTM);
		}
	}
	BuildBaseInstances();
}

void ACombatMapManager::BuildBaseInstances()
{
	if (!TilesISMC) return;

	TilesISMC->ClearInstances();
	for (const FTransform& TM : TilesTransformsLocal)
	{
		TilesISMC->AddInstance(TM);
	}
	
	HoverISMC->ClearInstances();
	PreviewISMC->ClearInstances();
	BlockedISMC->ClearInstances();
	MovementISMC->ClearInstances();
	CurrentHoveredInstance = INDEX_NONE;
}

void ACombatMapManager::SpawnUnit(const uint32 InUnitID, UStaticMesh* InStaticMesh, const FIntVector2& InUnitPos, const FUnitSize& InUnitSize,
	const FUnitRotation& InUnitRotation,  EUnitActorState InUnitActorState)
{
	FVector SpawnLoc = GridToLevelPosition(InUnitPos);
	SpawnLoc.Z += UnitZOffset;

	FActorSpawnParameters Params;
	Params.Owner = this;
		
	ABaseUnitActor* NewUnit =
			GetWorld()->SpawnActor<ABaseUnitActor>(
				UnitActorClass,
				SpawnLoc,
				FRotator(0, InUnitRotation.GetUnitFRotation(), 0),
				Params);
	NewUnit->SetID(InUnitID);
	NewUnit->SetUnitActorSize(InUnitSize);
	NewUnit->SetUnitActorRotation(InUnitRotation);
	NewUnit->SetUnitActorState(InUnitActorState);
	NewUnit->SetUnitMesh(InStaticMesh);
	NewUnit->ResizeMeshToSize(InUnitSize.GetUnitTileLength().X * GetTileSizeUU(), InUnitSize.GetUnitTileLength().Y * GetTileSizeUU());
	UnitActorsByID.Add(InUnitID, NewUnit);

	MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
				TEXT("Placing Unit {id = %d} at a Pos = {%s}"), NewUnit->GetID(), *SpawnLoc.ToString());
}

void ACombatMapManager::MoveUnitTo(const uint32 InUnitID, const FIntVector2& InUnitPos) const
{
	ABaseUnitActor* UnitActor = UnitActorsByID[InUnitID];
	UnitActor->SetUnitGridPosition(InUnitPos);
	
	FVector SpawnLoc = GridToLevelPosition(InUnitPos);
	SpawnLoc.Z += UnitZOffset;
	UnitActor->SetUnitWorldPosition(SpawnLoc);
}


void ACombatMapManager::DestroyGhostActor(ABaseUnitActor* InGhostActor)
{
	InGhostActor->Destroy();
	InGhostActor = nullptr;
}

ABaseUnitActor* ACombatMapManager::SpawnUnitActorGhost(const uint32 InBasedOnUnitActorID)
{
	ABaseUnitActor* BasedOnActor = UnitActorsByID[InBasedOnUnitActorID];
	
	FActorSpawnParameters Params;
	Params.Owner = this;

	FVector Loc = BasedOnActor->GetActorLocation();
	float Rot = BasedOnActor->GetUnitActorRotation().GetUnitFRotation();
	FUnitSize UnitSize = BasedOnActor->GetUnitActorSize();
	FUnitRotation Rotation = BasedOnActor->GetUnitActorRotation();
	UStaticMesh* Mesh = BasedOnActor->GetUnitStaticMesh();
	
	ABaseUnitActor* Ghost =
			GetWorld()->SpawnActor<ABaseUnitActor>(
				UnitActorClass,
				Loc,
				FRotator(0, Rot, 0),
				Params);
	
	Ghost->SetUnitActorSize(UnitSize);
	Ghost->SetUnitActorRotation(Rotation);
	Ghost->SetUnitActorState(EUnitActorState::Ghost);
	Ghost->SetUnitMesh(Mesh);
	Ghost->ResizeMeshToSize(UnitSize.GetUnitTileLength().X * GetTileSizeUU(), UnitSize.GetUnitTileLength().Y * GetTileSizeUU());

	MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
				TEXT("Placing Ghost at a Pos = {%s} based on unit id = %d"), *Loc.ToString(), InBasedOnUnitActorID);

	return Ghost;
}

void ACombatMapManager::SpawnUnitActors(const TArray<UUnitBase*>& ReplicatedUnits)
{	
	for (UUnitBase* Unit : ReplicatedUnits)
	{
		if (!Unit)
			continue;
		
		if (UnitActorsByID.Contains(Unit->GetUnitCombatID()))
			continue;

		FUnitActorEssentialInfo ActorEssentialInfo = Unit->GetUnitActorEssentialInfo();
		UStaticMesh* UnitMesh = GetStaticMeshComponentForUnitType(Unit->GetUnitTypeName());

		SpawnUnit(ActorEssentialInfo.UnitCombatID, UnitMesh, ActorEssentialInfo.UnitGridPosition,
			ActorEssentialInfo.UnitSize, ActorEssentialInfo.UnitRotation, EUnitActorState::Playable);
		
	}
	IsInitialUnitsSpawned = true;
}

bool ACombatMapManager::IsPositionForUnitAvailable(const uint32 InUnitID, const FIntVector2& InUnitCenter, TArray<FIntPoint>& OutBlockers) const
{
	ABaseUnitActor* UnitActor = UnitActorsByID[InUnitID];
	FUnitSize UnitSize = UnitActor->GetUnitActorSize();
	FUnitRotation UnitRotation = UnitActor->GetUnitActorRotation();
	bool IsAvailable = GetGameState()->CheckPositionForUnitWithCombatMap(InUnitCenter, UnitRotation, UnitSize, OutBlockers);
	return IsAvailable;
}


bool ACombatMapManager::IsPositionAvailable(const FUnitSize& InUnitSize, const FUnitRotation& UnitRotation,
	const FIntVector2& InUnitCenter, TArray<FIntPoint>& OutBlockers) const
{
	bool IsAvailable = GetGameState()->CheckPositionForUnitWithCombatMap(InUnitCenter, UnitRotation, InUnitSize, OutBlockers);
	return IsAvailable;
}

void ACombatMapManager::UpdateHoveredTile(int32 NewInstanceIndex)
{	
	if (NewInstanceIndex == CurrentHoveredInstance)
		return;

	CurrentHoveredInstance = (NewInstanceIndex >= 0 && TilesTransformsLocal.IsValidIndex(NewInstanceIndex))
								 ? NewInstanceIndex
								 : INDEX_NONE;

	SetHover(CurrentHoveredInstance);
	
	FTransform OutTransform;
	TilesISMC->GetInstanceTransform(NewInstanceIndex, OutTransform);
	// MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
	// 				TEXT("Hovering Over tile [%d] {%s} {%s}"), NewInstanceIndex,
	// 				*TileInstanceToGridPosition(NewInstanceIndex).ToString(),
	// 				*OutTransform.GetLocation().ToString());
}

void ACombatMapManager::SetHover(int32 TileIdx)
{
	HoverISMC->ClearInstances();

	if (TileIdx == INDEX_NONE || !TilesTransformsLocal.IsValidIndex(TileIdx))
		return;

	FTransform TM = TilesTransformsLocal[TileIdx];
	FVector L = TM.GetLocation();
	L.Z += HighlightZLift;
	TM.SetLocation(L);

	HoverISMC->AddInstance(TM);
}

void ACombatMapManager::ClearHover() const
{
	HoverISMC->ClearInstances();
}


void ACombatMapManager::UpdateVisualForBlockers(const TArray<FIntPoint>& InBlockers)
{
	if (InBlockers.Num() > 0)
	{	
		for (int i = 0; i < InBlockers.Num(); i++)
		{
			MG_COND_LOG(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
				TEXT("Tile = {%s} is blocking"), *InBlockers[i].ToString());
			SetBlocker(GridToTileIndex(InBlockers[i]));	
		}	
	}
	else
	{
		ClearBlocker();
	}
}

void ACombatMapManager::SetBlocker(const int32 TileIdx)
{
	BlockedISMC->ClearInstances();

	if (TileIdx == INDEX_NONE || !TilesTransformsLocal.IsValidIndex(TileIdx))
		return;

	FTransform TM = TilesTransformsLocal[TileIdx];
	FVector L = TM.GetLocation();
	L.Z += BlockerZLift;
	TM.SetLocation(L);

	BlockedISMC->AddInstance(TM);
}

void ACombatMapManager::ClearBlocker() const
{
	BlockedISMC->ClearInstances();
}

UStaticMesh* ACombatMapManager::GetStaticMeshComponentForUnitType(const FName& InUnitTypeName)
{
	FSoftObjectPath Path = GetUnitDataSubsystem(this)->GetUnitMeshObjectPath(InUnitTypeName);
	if (UStaticMesh* Mesh = Cast<UStaticMesh>(Path.TryLoad()))
	{
		return Mesh;
	}
	return nullptr;
}

UUnitDataSubsystem* ACombatMapManager::GetUnitDataSubsystem(const UObject* WorldContext)
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

FVector ACombatMapManager::CombatMapToLevelPosition(const FVector2f& InPos) const
{
	FVector Res = GridOrigin + FVector(InPos.X * MetresToUUScale,InPos.Y * MetresToUUScale, 0.f);
	return Res;
}

FIntVector2 ACombatMapManager::CalculateDistanceToForUnitID(const uint32 InUnitID,
	const FIntVector2& InGridPosition) const
{
	ABaseUnitActor* A = UnitActorsByID[InUnitID];
	if (!A)
	{
		MG_COND_ERROR(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
				TEXT("Could not find Unit with ID = {%d}"), InUnitID);
		return FIntVector2::NoneValue;
	}
	return (A->GetUnitGridPosition() - InGridPosition);
}

FVector ACombatMapManager::GridToLevelPosition(const FIntVector2& TileGridCoords) const
{
	const float Half = TileSize * 0.5f;
	FVector Res = GridOrigin + FVector(TileGridCoords.X * TileSize + Half, TileGridCoords.Y * TileSize + Half,0.f);
	return Res;
}

FVector ACombatMapManager::GetUnitWorldPositionByID(const uint32 InUnitID) const
{
	ABaseUnitActor* A = UnitActorsByID[InUnitID];
	if (!A)
	{
		MG_COND_ERROR(ACombatMapManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::CombatMapManager),
				TEXT("Could not find Unit with ID = {%d}"), InUnitID);
		return FVector::ZeroVector;
	}
	return A->GetUnitWorldPosition();
		
}

int32 ACombatMapManager::GridToTileIndex(const FIntPoint& InGridPos) const
{
	return InGridPos.X * GridSize + InGridPos.Y;
}


FIntVector2 ACombatMapManager::TileInstanceToGridPosition(const int32 TileInstanceIndex) const
{
	if (TileInstanceIndex < 0 || GridSize == 0)
		return FIntVector2(-1, -1);

	const int32 X = TileInstanceIndex / GridSize;
	const int32 Y = TileInstanceIndex % GridSize;
	return FIntVector2(X, Y);
}

AWarpGameState* ACombatMapManager::GetGameState() const
{
	if (AWarpGameState* GameState = GetWorld()->GetGameState<AWarpGameState>())
	{
		return GameState;
	}
	return nullptr;
}
