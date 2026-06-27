// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraUserFacingExperienceDefinition.h"
#include "LyraFPSUserFacingExperienceDefinition.generated.h"

class AGameModeBase;

/**
 * User-facing experience that can override GameMode through the travel URL.
 */
UCLASS(BlueprintType)
class LYRAFPSGAMERUNTIME_API ULyraFPSUserFacingExperienceDefinition : public ULyraUserFacingExperienceDefinition
{
	GENERATED_BODY()

public:
	ULyraFPSUserFacingExperienceDefinition();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual UCommonSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	TSoftClassPtr<AGameModeBase> GameModeOverride;
};
