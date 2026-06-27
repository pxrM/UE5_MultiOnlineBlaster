// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerState.h"
#include "LyraFPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ALyraFPSPlayerState : public ALyraPlayerState
{
	GENERATED_BODY()
public:
	ALyraFPSPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
