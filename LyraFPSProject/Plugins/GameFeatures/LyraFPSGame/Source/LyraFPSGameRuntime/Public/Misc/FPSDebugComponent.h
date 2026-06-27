// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "FPSDebugComponent.generated.h"

/** Character-only debug helper for local network stats. */
UCLASS(Blueprintable, ClassGroup=(LyraFPS), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API UFPSDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFPSDebugComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category="LyraFPS|Debug")
	bool GetNetworkDebugValues(float& PingMilliseconds, float& InLossPercent, float& OutLossPercent, bool bUseAverageLoss = true) const;

	UFUNCTION(BlueprintCallable, Category="LyraFPS|Debug")
	void PrintNetworkDebugString(bool bUseAverageLoss = true, float Duration = 0.0f) const;

private:
	class APlayerController* GetOwnerPlayerController() const;
	class UNetConnection* GetOwnerNetConnection(const APlayerController* PlayerController) const;
};
