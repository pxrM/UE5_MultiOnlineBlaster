// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "Layout/Margin.h"
#include "UIPanelStyle.generated.h"

/**
 * Data-driven panel / container appearance (background + inner padding).
 * See Docs/DESIGN.md section 5.
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIPanelStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Panel")
	FSlateBrush Background;

	/** Padding between the panel border and its content. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Panel")
	FMargin ContentPadding = FMargin(8.f);
};
