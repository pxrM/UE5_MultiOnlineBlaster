// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/GameMode/LyraFPSGameMode.h"

#include "Gameplay/LyraFPSPlayerState.h"

ALyraFPSGameMode::ALyraFPSGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerStateClass = ALyraFPSPlayerState::StaticClass();
}
