// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacePointer.h"


// Sets default values
APlacePointer::APlacePointer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Root_ = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root_);

	Mesh_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh_->SetupAttachment(Root_);
}

// Called when the game starts or when spawned
void APlacePointer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlacePointer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

