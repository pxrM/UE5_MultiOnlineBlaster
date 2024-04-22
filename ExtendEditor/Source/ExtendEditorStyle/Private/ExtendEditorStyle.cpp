// ExtendEditorStyle.cpp

#pragma once

#include "ExtendEditorStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"


IMPLEMENT_MODULE(FExtendEditorStyle, ExtendEditorStyle)

void FExtendEditorStyle::StartupModule()
{
	Initialize();
}

void FExtendEditorStyle::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
	StyleSet.Reset();
}

void FExtendEditorStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

TSharedRef<class FSlateStyleSet> FExtendEditorStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> SlateStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
	const FString RootPath = FPaths::ProjectDir() + TEXT("/Resource/");
	SlateStyleSet->SetContentRoot(RootPath);
	{
		const FVector2D IconeSize(16.f, 16.f);
		FSlateImageBrush* SlateImageBrush = new FSlateImageBrush(RootPath + TEXT("AliceIcon.png"), IconeSize);
		SlateStyleSet->Set("AliceTool", SlateImageBrush);
	}
	return SlateStyleSet;
}

EXTENDEDITORSTYLE_API FName FExtendEditorStyle::GetStyleSetName()
{
	return StyleSetName;
}
