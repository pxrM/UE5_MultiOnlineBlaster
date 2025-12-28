#include "UMGReflectorStyle.h"

#include "Components/Image.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUMGReflectorStyle::StyleInstance = nullptr;

void FUMGReflectorStyle::Initialize()
{
	if (StyleInstance.IsValid() == false)
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUMGReflectorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

void FUMGReflectorStyle::ReloadTextureResources()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUMGReflectorStyle::Get()
{
	return *StyleInstance;
}

FName FUMGReflectorStyle::GetStyleName()
{
	static const FName StyleName(TEXT("UMGReflectorStyle"));
	return StyleName;
}

TSharedPtr<FSlateStyleSet> FUMGReflectorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UMGReflector")->GetBaseDir() / TEXT("Resources"));
	Style->Set("UMGReflector.OpenPluginWindow", new IMAGE_BRUSH(TEXT("Icon128"), FVector2D(15.0f, 15.0f)));
	return Style;
}
