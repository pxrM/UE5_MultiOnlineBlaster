// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIDialogBase.generated.h"

class UTextBlock;
class UUIButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIDialogConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIDialogCancelled);

/**
 * Modal confirm dialog. Pushed onto the Modal layer via UUILayerSubsystem; closes
 * itself by popping that layer, so the stack stays in sync. Demonstrates the layer
 * system + button controls working together. See Docs/DESIGN.md sections 4 & 6.
 *
 * WBP pairing: reparent WBP_Dialog to this class. Name the title/body text blocks
 * "TitleText" / "BodyText" and the buttons "ConfirmButton" / "CancelButton"
 * (buttons reparented to UUIButtonBase).
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIDialogBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fired when confirm is pressed (before the dialog closes). */
	UPROPERTY(BlueprintAssignable, Category = "UI|Dialog")
	FUIDialogConfirmed OnConfirmed;

	/** Fired when cancel is pressed or the dialog is dismissed. */
	UPROPERTY(BlueprintAssignable, Category = "UI|Dialog")
	FUIDialogCancelled OnCancelled;

	/** Fill in the title and body text. */
	UFUNCTION(BlueprintCallable, Category = "UI|Dialog")
	void Setup(const FText& InTitle, const FText& InBody);

	/** Close the dialog by popping the Modal layer. */
	UFUNCTION(BlueprintCallable, Category = "UI|Dialog")
	void Close();

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleConfirm();

	UFUNCTION()
	void HandleCancel();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Dialog")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Dialog")
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Dialog")
	TObjectPtr<UUIButtonBase> ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Dialog")
	TObjectPtr<UUIButtonBase> CancelButton;
};
