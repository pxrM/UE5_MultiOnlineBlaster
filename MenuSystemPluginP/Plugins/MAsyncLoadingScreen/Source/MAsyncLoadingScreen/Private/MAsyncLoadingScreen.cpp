// Copyright Epic Games, Inc. All Rights Reserved.

#include "MAsyncLoadingScreen.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenFuncLibrary.h"
#include "SClassicLayout.h"
#include "MoviePlayer.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "FMAsyncLoadingScreenModule"

void FMAsyncLoadingScreenModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// 
	// ��ǰ�Ƿ�������ר�÷�������IsRunningDedicatedServer()�������� Slate Ӧ�ó����Ѿ���ʼ����FSlateApplication::IsInitialized()����
	if (!IsRunningDedicatedServer() && FSlateApplication::IsInitialized())
	{
		// ��ȡĬ�ϵ� ULoadingScreenSettings ���ö���
		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();

		if (IsMoviePlayerEnabled())
		{
			// ����Ӱ������׼����ʾ���س���ʱ���ûص������������á�
			GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, &FMAsyncLoadingScreenModule::PreSetupLoadingScreen);
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
	if (!IsRunningDedicatedServer())
	{
		// �Ƴ������뵱ǰ����this����ص� OnPrepareLoadingScreen �¼��İ�
		GetMoviePlayer()->OnPrepareLoadingScreen().RemoveAll(this);
	}
}

bool FMAsyncLoadingScreenModule::IsGameModule() const
{
	return true;
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
	RemoveAllBackgroundImages();

	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();

	for (auto& Image : Settings->StartupLoadingScreen.Background.Images)
	{
		UTexture2D* LoadedImage = Cast<UTexture2D>(Image.TryLoad());
		if (LoadedImage)
		{
			StartupBackgroundImages.Add(LoadedImage);
		}
	}

	for (auto& Image : Settings->DefaultLoadingScreen.Background.Images)
	{
		UTexture2D* LoadedImage = Cast<UTexture2D>(Image.TryLoad());
		if (LoadedImage)
		{
			DefaultBackgroundImages.Add(LoadedImage);
		}
	}
}

void FMAsyncLoadingScreenModule::RemoveAllBackgroundImages()
{
	StartupBackgroundImages.Empty();
	DefaultBackgroundImages.Empty();
}

void FMAsyncLoadingScreenModule::PreSetupLoadingScreen()
{
	UE_LOG(LogTemp, Warning, TEXT("PreSetupLoadingScreen"));
	const bool bIsEnableLoadingScreen = UAsyncLoadingScreenFuncLibrary::GetIsEnableLoadingScreen();
	if (bIsEnableLoadingScreen)
	{
		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
		bIsStartupLoadingScreen = false;
		SetupLoadingScreen(Settings->DefaultLoadingScreen);
	}
}

void FMAsyncLoadingScreenModule::SetupLoadingScreen(const FALoadingScreenSettings& LoadingScreenSettings)
{
	TArray<FString> MoviesList = LoadingScreenSettings.MoviePaths;

	if (LoadingScreenSettings.bShuffle)
	{
		ShuffleMovies(MoviesList);
	}

	if (LoadingScreenSettings.bSetDisplayMoviesIndexManually == true)
	{
		MoviesList.Empty();

		if (LoadingScreenSettings.MoviePaths.IsValidIndex(UAsyncLoadingScreenFuncLibrary::GetDisplayMovieIndex()))
		{
			MoviesList.Add(LoadingScreenSettings.MoviePaths[UAsyncLoadingScreenFuncLibrary::GetDisplayMovieIndex()]);
		}
		else
		{
			MoviesList = LoadingScreenSettings.MoviePaths;
		}
	}

	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.MinimumLoadingScreenDisplayTime = LoadingScreenSettings.MinimumLoadingScreenDisplayTime;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = LoadingScreenSettings.bAutoCompleteWhenLoadingCompletes;
	LoadingScreen.bMoviesAreSkippable = LoadingScreenSettings.bMoviesAreSkippable;
	LoadingScreen.bWaitForManualStop = LoadingScreenSettings.bWaitForManualStop;
	LoadingScreen.bAllowInEarlyStartup = LoadingScreenSettings.bAllowInEarlyStartup;
	LoadingScreen.bAllowEngineTick = LoadingScreenSettings.bAllowEngineTick;
	LoadingScreen.MoviePaths = MoviesList;
	LoadingScreen.PlaybackType = LoadingScreenSettings.PlaybackType;

	if (LoadingScreenSettings.bShowWidgetOverlay)
	{
		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
		switch (Settings->Layout)
		{
		case EAsyncLoadingScreenLayout::ALSL_Classic:
			LoadingScreen.WidgetLoadingScreen = SNew(SClassicLayout, LoadingScreenSettings, Settings->Classic);
			break;
		default:
			break;
		}
	}

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void FMAsyncLoadingScreenModule::ShuffleMovies(TArray<FString>& MoviesList)
{
	if (MoviesList.Num() > 0)
	{
		int32 LastIndex = MoviesList.Num() - 1;
		for (int32 i = 0; i < MoviesList.Num() - 1; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				MoviesList.Swap(i, Index);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMAsyncLoadingScreenModule, MAsyncLoadingScreen)