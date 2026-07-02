// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "Layout/Margin.h"
#include "UIButtonStyle.generated.h"

class UUITextStyle;

/**
 * Data-driven button appearance across interaction states. Controls read this
 * instead of hardcoding per-state brushes. See Docs/DESIGN.md section 5.
 * (CommonUI ships UCommonButtonStyle with the same intent; extend that if you
 * are already committed to CommonUI buttons.)
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIButtonStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button|State")
	FSlateBrush Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button|State")
	FSlateBrush Hovered;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button|State")
	FSlateBrush Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button|State")
	FSlateBrush Disabled;

	/** Padding between the button border and its content. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button")
	FMargin ContentPadding = FMargin(12.f, 6.f);

	/** Text style applied to the button label. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button")
	TObjectPtr<UUITextStyle> TextStyle;
};
