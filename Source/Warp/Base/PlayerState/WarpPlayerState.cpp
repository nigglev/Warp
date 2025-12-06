// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayerState.h"

void AWarpPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWarpPlayerState, bClientContentReady);
}
