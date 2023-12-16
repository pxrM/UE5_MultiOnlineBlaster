// Copyright Epic Games, Inc. All Rights Reserved.

#include "MAsyncLoadingScreen.h"
#include "LoadingScreenSettings.h"
#include "SClassicLayout.h"
#include "MoviePlayer.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "FMAsyncLoadingScreenModule"

void FMAsyncLoadingScreenModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// 
	// 当前是否不在运行专用服务器（IsRunningDedicatedServer()），并且 Slate 应用程序已经初始化（FSlateApplication::IsInitialized()）。
	if (!IsRunningDedicatedServer() && FSlateApplication::IsInitialized())
	{
		// 获取默认的 ULoadingScreenSettings 设置对象。
		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();

		if (IsMoviePlayerEnabled())
		{
			// 当电影播放器准备显示加载场景时，该回调函数将被调用。
			GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, FMAsyncLoadingScreenModule::PreSetupLoadingScreen);
		}

		if (Settings->bPerloadBackgroundImage)
		{
			LoadBackgroundImages();
		}

		bIsStartupLoadingScreen = true;
		SetupLoadingScreen(Settings->StartupLoadingScreen);
	}
}

void FMAsyncLoadingScreenModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FMAsyncLoadingScreenModule::IsGameModule() const
{
	return false;
}

TArray<UTexture2D*> FMAsyncLoadingScreenModule::GetBackgroundImages()
{
	return bIsStartupLoadingScreen ? StartupBackgroundImages : DefaultBackgroundImages;
}

bool FMAsyncLoadingScreenModule::IsPreloadBackgroundImageEnable()
{
	return GetDefault<ULoadingScreenSettings>()->bPerloadBackgroundImage;
}

void FMAsyncLoadingScreenModule::LoadBackgroundImages()
{

}

void FMAsyncLoadingScreenModule::RemoveAllBackgroundImages()
{

}

void FMAsyncLoadingScreenModule::PreSetupLoadingScreen()
{
}

void FMAsyncLoadingScreenModule::SetupLoadingScreen(const FALoadingScreenSettings& LoadingScreenSettings)
{
}

void FMAsyncLoadingScreenModule::ShuffleMovies(TArray<FString>& MoviesList)
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMAsyncLoadingScreenModule, MAsyncLoadingScreen)