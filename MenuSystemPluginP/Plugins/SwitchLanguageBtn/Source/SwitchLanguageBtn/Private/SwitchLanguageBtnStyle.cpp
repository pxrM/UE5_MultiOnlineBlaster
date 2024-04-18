// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwitchLanguageBtnStyle.h"
#include "SwitchLanguageBtn.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSwitchLanguageBtnStyle::StyleInstance = nullptr;

void FSwitchLanguageBtnStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSwitchLanguageBtnStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSwitchLanguageBtnStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SwitchLanguageBtnStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FSwitchLanguageBtnStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("SwitchLanguageBtnStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SwitchLanguageBtn")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SwitchLanguageBtn.PluginAction", new IMAGE_BRUSH(TEXT("SwitchLanguageButton"), Icon20x20));
	return Style;
}

void FSwitchLanguageBtnStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSwitchLanguageBtnStyle::Get()
{
	return *StyleInstance;
}
