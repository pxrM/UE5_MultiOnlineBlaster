// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FAVSStyle
{
public:

	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:

	static TSharedPtr<FSlateStyleSet> StyleInstance;
	static constexpr const TCHAR* DragDots_ElementPNG = TEXT("DragDots_Element.png");

};