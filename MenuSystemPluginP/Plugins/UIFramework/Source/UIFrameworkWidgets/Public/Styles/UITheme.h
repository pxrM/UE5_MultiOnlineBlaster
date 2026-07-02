// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UITheme.generated.h"

class UUIButtonStyle;
class UUITextStyle;
class UUIPanelStyle;

/**
 * A theme bundles every style asset plus a shared color palette. Swap the active
 * theme at runtime (DA_Theme_Dark / DA_Theme_Light) to restyle the whole UI.
 * See Docs/DESIGN.md section 5.
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUITheme : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Style assets ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Styles")
	TObjectPtr<UUIButtonStyle> ButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Styles")
	TObjectPtr<UUITextStyle> TextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Styles")
	TObjectPtr<UUIPanelStyle> PanelStyle;

	// --- Palette tokens. Reference these for one-off tints so colors stay centralized. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Primary = FLinearColor(0.1f, 0.5f, 0.9f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Secondary = FLinearColor(0.2f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Background = FLinearColor(0.05f, 0.05f, 0.05f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Success = FLinearColor(0.2f, 0.8f, 0.3f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Warning = FLinearColor(0.9f, 0.7f, 0.1f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	FLinearColor Danger = FLinearColor(0.9f, 0.2f, 0.2f, 1.f);
};
