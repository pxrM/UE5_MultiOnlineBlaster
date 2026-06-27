// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSStyle.h"
#include "FAVSSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"


class UFAVSSettings;
TSharedPtr<FSlateStyleSet> FAVSStyle::StyleInstance = nullptr;
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

void FAVSStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = MakeShareable(new FSlateStyleSet("FAVSStyle"));

		const FString ResourcesDir = IPluginManager::Get().FindPlugin(TEXT("FAVS"))->GetBaseDir() / TEXT("Resources");
		StyleInstance->SetContentRoot(ResourcesDir);

		StyleInstance->Set("DraggableHandle_Element", new FSlateImageBrush(ResourcesDir / DragDots_ElementPNG, FVector2D(16, 22)));
		StyleInstance->Set("FAVS.OpenPluginWindow", new FSlateVectorImageBrush(ResourcesDir / TEXT("PlaceholderButtonIcon.svg"), Icon20x20));
		StyleInstance->Set("Toggle_ON", new FSlateImageBrush(ResourcesDir / TEXT("Toggle_ON.png"), FVector2D(45, 24)));
		StyleInstance->Set("Toggle_OFF", new FSlateImageBrush(ResourcesDir / TEXT("Toggle_OFF.png"), FVector2D(45, 24)));
		FButtonStyle ButtonStyle = FButtonStyle()
			.SetNormal(   FSlateImageBrush(ResourcesDir / TEXT("BTN_Up.png"),   FVector2D(72,24)))
			.SetHovered(  FSlateImageBrush(ResourcesDir / TEXT("BTN_Over.png"),  FVector2D(72,24)))
			.SetPressed(  FSlateImageBrush(ResourcesDir / TEXT("BTN_Down.png"),  FVector2D(72,24)))
			.SetNormalPadding(FMargin(0))
			.SetPressedPadding(FMargin(0,0,0,0));

		StyleInstance->Set("ButtonAdd", ButtonStyle);

		FButtonStyle ZwinietyBtnStyle = FButtonStyle()
			   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("BTN_Zwiniete.png"),   FVector2D(18,15)))
			   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("BTN_Zwiniete_Over.png"), FVector2D(18,15)))
			   .SetPressed(  FSlateImageBrush(ResourcesDir / TEXT("BTN_Zwiniete_Over.png"),  FVector2D(18,15)))
			   .SetNormalPadding(FMargin(0))
			   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("ButtonZwiniety", ZwinietyBtnStyle);
		
		FButtonStyle RozwinietyBtnStyle = FButtonStyle()
			   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("BTN_Rozwiniete.png"),   FVector2D(18,15)))
			   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("BTN_Rozwiniete_Over.png"), FVector2D(18,15)))
			   .SetPressed (FSlateImageBrush(ResourcesDir / TEXT("BTN_Rozwiniete_Over.png"), FVector2D(18,15)))
			   .SetNormalPadding(FMargin(0))
			   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("ButtonRozwiniety", RozwinietyBtnStyle);

		FButtonStyle OpenAssetBtn = FButtonStyle()
					   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn.png"),   FVector2D(30,20)))
					   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn_Over.png"), FVector2D(30,20)))
					   .SetPressed (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn_Over.png"), FVector2D(30,20)))
					   .SetNormalPadding(FMargin(0))
					   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("OpenAssetBtn", OpenAssetBtn);

		FButtonStyle OpenAssetBtn_Opened = FButtonStyle()
					   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn_Opened.png"),   FVector2D(30,20)))
					   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn_Over.png"), FVector2D(30,20)))
					   .SetPressed (FSlateImageBrush(ResourcesDir / TEXT("OpenBtn_Over.png"), FVector2D(30,20)))
					   .SetNormalPadding(FMargin(0))
					   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("OpenAssetBtn_Opened", OpenAssetBtn_Opened);
		
		FButtonStyle OpenInCBBtn = FButtonStyle()
							   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("BTN_OpenInCB.png"),   FVector2D(26,24)))
							   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("BTN_OpenInCB_Over.png"), FVector2D(26,24)))
							   .SetPressed (FSlateImageBrush(ResourcesDir / TEXT("BTN_OpenInCB_Over.png"), FVector2D(26,24)))
							   .SetNormalPadding(FMargin(0))
							   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("OpenInCBBtn", OpenInCBBtn);
		
		FButtonStyle RemoveBtn = FButtonStyle()
					   .SetNormal  (FSlateImageBrush(ResourcesDir / TEXT("BTN_Remove.png"),   FVector2D(20,20)))
					   .SetHovered (FSlateImageBrush(ResourcesDir / TEXT("BTN_Remove_Over.png"), FVector2D(20,20)))
					   .SetPressed (FSlateImageBrush(ResourcesDir / TEXT("BTN_Remove_Over.png"), FVector2D(20,20)))
					   .SetNormalPadding(FMargin(0))
					   .SetPressedPadding(FMargin(0));
		StyleInstance->Set("RemoveBtn", RemoveBtn);

		FSlateImageBrush* WrongAsset = new FSlateImageBrush(ResourcesDir / TEXT("WrongAsset.png"),FVector2D(23, 22));
		StyleInstance->Set("WrongAsset", WrongAsset);

		FSlateImageBrush* SegmentLeftBrush = new FSlateImageBrush(ResourcesDir / TEXT("SegmentBackground_Left.png"),FVector2D(5, 38));
		StyleInstance->Set("SegmentBackground_Left", SegmentLeftBrush);

		FSlateImageBrush* SegmentCenterBrush = new FSlateImageBrush(ResourcesDir / TEXT("SegmentBackground_Center.png"),FVector2D(4, 38));
		StyleInstance->Set("SegmentBackground_Center", SegmentCenterBrush);

		FSlateImageBrush* SegmentRightBrush = new FSlateImageBrush(ResourcesDir / TEXT("SegmentBackground_Right.png"),FVector2D(5, 38));
		StyleInstance->Set("SegmentBackground_Right", SegmentRightBrush);

		FSlateImageBrush* ElementStrip_Left = new FSlateImageBrush(ResourcesDir / TEXT("ElementPasek_Left.png"),FVector2D(7, 7));
		StyleInstance->Set("ElementStrip_Left", ElementStrip_Left);

		FSlateImageBrush* ElementStrip_Center = new FSlateImageBrush(ResourcesDir / TEXT("ElementPasek_Center.png"),FVector2D(3, 7));
		StyleInstance->Set("ElementStrip_Center", ElementStrip_Center);

		FSlateImageBrush* ElementStrip_Right = new FSlateImageBrush(ResourcesDir / TEXT("ElementPasek_Right.png"),FVector2D(7, 7));
		StyleInstance->Set("ElementStrip_Right", ElementStrip_Right);

		const FSlateBrush* BaseBrush = FAppStyle::Get().GetBrush("Icons.Star");
		FSlateBrush* StarBrush = new FSlateBrush(*BaseBrush);

		StarBrush->TintColor = FSlateColor(GetDefault<UFAVSSettings>()->IconColor);

		StyleInstance->Set("FAVS.StarTint", StarBrush);
		
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAVSStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAVSStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FAVSStyle"));
	return StyleSetName;
}

void FAVSStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAVSStyle::Get()
{
	return *StyleInstance;
}
