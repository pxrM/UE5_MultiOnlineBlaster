// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIButtonBase.generated.h"

class UButton;
class UTextBlock;
class UUIButtonStyle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIButtonClicked);

/**
 * Button control that pulls per-state brushes and label style from a UUIButtonStyle
 * asset. Reapplies on PreConstruct for editor preview and re-broadcasts the inner
 * button's click as OnClicked. See Docs/DESIGN.md sections 5-6.
 *
 * WBP pairing: reparent WBP_Button to this class, name the inner Button "Button"
 * and the optional label Text Block "Label".
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIButtonBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fired when the button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "UI|Button")
	FUIButtonClicked OnClicked;

	/** Set the button label text. */
	UFUNCTION(BlueprintCallable, Category = "UI|Button")
	void SetLabel(const FText& InText);

	/** Swap the style asset and reapply. */
	UFUNCTION(BlueprintCallable, Category = "UI|Button")
	void SetStyleAsset(UUIButtonStyle* InStyle);

	/** Enable / disable the button. */
	UFUNCTION(BlueprintCallable, Category = "UI|Button")
	void SetButtonEnabled(bool bEnabled);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;

	/** Push the style asset's values onto the bound button and label. */
	void ApplyStyle();

	UFUNCTION()
	void HandleClicked();

	/** Bound inner button. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Button")
	TObjectPtr<UButton> Button;

	/** Optional label text block. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Button")
	TObjectPtr<UTextBlock> Label;

	/** Style source. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Button")
	TObjectPtr<UUIButtonStyle> Style;

	/** Design-time / default label text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Button")
	FText DefaultLabel;
};
