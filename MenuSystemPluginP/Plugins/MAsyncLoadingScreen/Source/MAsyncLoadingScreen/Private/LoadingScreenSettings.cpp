// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingScreenSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"

#define LOCTEXT_NAMESPACE "MAsyncLoadingScreen"

ULoadingScreenSettings::ULoadingScreenSettings(const FObjectInitializer& ObjecInitializer) :Super(ObjecInitializer)
{
	StartupLoadingScreen.TipWidget.TipWrapAt = 1000.0f;
	StartupLoadingScreen.bShowWidgetOverlay = false;
	DefaultLoadingScreen.TipWidget.TipWrapAt = 1000.0f;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(TEXT("/Engine/EngineFonts/Roboto"));
		StartupLoadingScreen.TipWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Normal"));
		DefaultLoadingScreen.TipWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Normal"));
		StartupLoadingScreen.LoadingWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 32, FName("Normal"));
		DefaultLoadingScreen.LoadingWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 32, FName("Normal"));
		StartupLoadingScreen.LoadingCompleteTextSettings.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Normal"));
		DefaultLoadingScreen.LoadingCompleteTextSettings.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Normal"));
	}
}

FLoadingWidgetSettings::FLoadingWidgetSettings() : LoadingText(LOCTEXT("Loading", "LOADING"))
{
}