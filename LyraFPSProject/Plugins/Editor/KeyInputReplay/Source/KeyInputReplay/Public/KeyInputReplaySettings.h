// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/DeveloperSettings.h"
#include "KeyInputReplaySettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Key Input Replay"))
class KEYINPUTREPLAY_API UKeyInputReplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Record")
	TArray<FKey> IgnoreRecordKeys = {FKey(FName(TEXT("@")))};
};