// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UITextBase.generated.h"

class UTextBlock;
class UUITextStyle;

/**
 * Text control that pulls its appearance from a UUITextStyle asset instead of
 * hardcoding font/color in the WBP. Reapplies on PreConstruct so the editor
 * preview tracks style edits. See Docs/DESIGN.md sections 5-6.
 *
 * WBP pairing: reparent WBP_Text to this class and name the inner Text Block "Text".
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUITextBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the displayed text. */
	UFUNCTION(BlueprintCallable, Category = "UI|Text")
	void SetText(const FText& InText);

	/** Swap the style asset and reapply. */
	UFUNCTION(BlueprintCallable, Category = "UI|Text")
	void SetStyleAsset(UUITextStyle* InStyle);

protected:
	virtual void NativePreConstruct() override;

	/** Push the style asset's values onto the bound text block. */
	void ApplyStyle();

	/** Bound inner text block. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Text")
	TObjectPtr<UTextBlock> Text;

	/** Style source. Change in the WBP defaults or at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Text")
	TObjectPtr<UUITextStyle> Style;

	/** Design-time / default text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Text")
	FText DefaultText;
};
