// Copyright Epic Games, Inc. All Rights Reserved.

#include "MAsyncLoadingScreen.h"
#include "LoadingScreenSettings.h"

#define LOCTEXT_NAMESPACE "FMAsyncLoadingScreenModule"

void FMAsyncLoadingScreenModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMAsyncLoadingScreenModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FMAsyncLoadingScreenModule::IsPreloadBackgroundImageEnable()
{
	return GetDefault<ULoadingScreenSettings>()->bPerloadBackgroundImage;
}

TArray<UTexture2D*> FMAsyncLoadingScreenModule::GetBackgroundImages()
{
	return bIsStartupLoadingScreen ? StartupBackgroundImages : DefaultBackgroundImages;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMAsyncLoadingScreenModule, MAsyncLoadingScreen)