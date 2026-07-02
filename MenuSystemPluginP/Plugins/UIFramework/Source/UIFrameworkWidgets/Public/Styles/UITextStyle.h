// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateColor.h"
#include "UITextStyle.generated.h"

/**
 * Data-driven text appearance. Controls read this instead of hardcoding font/color,
 * so a theme swap restyles every text widget at once. See Docs/DESIGN.md section 5.
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUITextStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	FSlateColor Color = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	FVector2D ShadowOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	FLinearColor ShadowColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	/** Extra spacing between characters, in points. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	int32 LetterSpacing = 0;
};
