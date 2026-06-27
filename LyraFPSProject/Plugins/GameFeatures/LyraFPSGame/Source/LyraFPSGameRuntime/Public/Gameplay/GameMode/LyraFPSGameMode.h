// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "LyraFPSGameMode.generated.h"

/**
 * Lyra FPS gameplay mode.
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ALyraFPSGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:
	ALyraFPSGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
