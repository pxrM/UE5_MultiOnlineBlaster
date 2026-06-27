// Fill out your copyright notice in the Description page of Project Settings.
#include "Gameplay/GameMode/LyraFPSUserFacingExperienceDefinition.h"
#include "CommonSessionSubsystem.h"
#include "Gameplay/GameMode/LyraFPSGameMode.h"

ULyraFPSUserFacingExperienceDefinition::ULyraFPSUserFacingExperienceDefinition()
{
	GameModeOverride = ALyraFPSGameMode::StaticClass();
}

FPrimaryAssetId ULyraFPSUserFacingExperienceDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(ULyraUserFacingExperienceDefinition::StaticClass()->GetFName(), GetFName());
}

UCommonSession_HostSessionRequest* ULyraFPSUserFacingExperienceDefinition::CreateHostingRequest(const UObject* WorldContextObject) const
{
	UCommonSession_HostSessionRequest* Result = Super::CreateHostingRequest(WorldContextObject);
	if (Result && !GameModeOverride.IsNull())
	{
		Result->ExtraArgs.Add(TEXT("game"), GameModeOverride.ToSoftObjectPath().ToString());
	}

	return Result;
}
