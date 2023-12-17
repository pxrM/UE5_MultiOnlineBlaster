// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncLoadingScreenFuncLibrary.h"
#include "MoviePlayer.h"
#include "MAsyncLoadingScreen.h"


int32 UAsyncLoadingScreenFuncLibrary::DisplayBackgroundIndex = -1;
int32 UAsyncLoadingScreenFuncLibrary::DisplayTipTextIndex = -1;
int32 UAsyncLoadingScreenFuncLibrary::DisplayMovieIndex = -1;
bool  UAsyncLoadingScreenFuncLibrary::bShowLoadingScreen = true;

void UAsyncLoadingScreenFuncLibrary::SetDisplayBackgroundIndex(int32 BackgroundIndex)
{
	UAsyncLoadingScreenFuncLibrary::DisplayBackgroundIndex = BackgroundIndex;
}

void UAsyncLoadingScreenFuncLibrary::SetDisplayTipTextIndex(int32 TipTextIndex)
{
	UAsyncLoadingScreenFuncLibrary::DisplayTipTextIndex = TipTextIndex;
}

void UAsyncLoadingScreenFuncLibrary::SetDisplayMovieIndex(int32 MovieIndex)
{
	UAsyncLoadingScreenFuncLibrary::DisplayMovieIndex = MovieIndex;
}

void UAsyncLoadingScreenFuncLibrary::SetEnableLoadingScreen(bool bIsEnableLoadingScreen)
{
	bShowLoadingScreen = bIsEnableLoadingScreen;
}

void UAsyncLoadingScreenFuncLibrary::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void UAsyncLoadingScreenFuncLibrary::PreloadBackgroundImages()
{
	if (FMAsyncLoadingScreenModule::IsAvailable())
	{
		FMAsyncLoadingScreenModule& LoadingScreenModule = FMAsyncLoadingScreenModule::Get();
		if (LoadingScreenModule.IsPreloadBackgroundImageEnable())
		{
			LoadingScreenModule.LoadBackgroundImages();
		}
	}
}

void UAsyncLoadingScreenFuncLibrary::RemovePreloadedBackgroundImages()
{
	if (FMAsyncLoadingScreenModule::IsAvailable())
	{
		FMAsyncLoadingScreenModule& LoadingScreenModule = FMAsyncLoadingScreenModule::Get();
		LoadingScreenModule.RemoveAllBackgroundImages();
	}
}
