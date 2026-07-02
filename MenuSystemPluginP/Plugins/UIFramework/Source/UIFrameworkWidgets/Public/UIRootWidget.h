// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UILayerTypes.h"
#include "UIRootWidget.generated.h"

class UPanelWidget;

/**
 * Root widget for the whole UI. One instance lives in the viewport.
 *
 * Design: each layer maps to a bound container panel inside the WBP (stacked by an
 * Overlay so child order defines Z). Pages are never added to a container directly;
 * they go through PushToLayer / PopFromLayer so activation, back handling and focus
 * stay managed. See Docs/DESIGN.md section 4.
 *
 * WBP usage: reparent the root WBP to this class, then name the per-layer containers
 * to match the BindWidget properties below (Layer_GameHUD, Layer_Menu, ...).
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Push a widget onto the given layer's stack. Returns the created widget (top of stack). */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass);

	/** Pop the top widget off the given layer. Returns true if a widget was removed. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool PopFromLayer(EUILayer Layer);

	/** Remove every widget on the given layer. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void PopAllFromLayer(EUILayer Layer);

	/** Remove a specific widget from a layer's stack (not necessarily the top). Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool RemoveWidget(EUILayer Layer, UUserWidget* Widget);

	/** Top widget of a layer, or nullptr if the layer is empty. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* GetTopWidget(EUILayer Layer) const;

	/** True if the layer has at least one widget. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool IsLayerActive(EUILayer Layer) const;

	/**
	 * Handle a back / cancel action. Pops the top-most widget of the highest active
	 * layer (Tooltip first, GameHUD last). Returns true if something was consumed.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool HandleBackAction();

protected:
	virtual void NativeOnInitialized() override;

	/** Resolve the bound container panel for a layer, or nullptr if not bound. */
	UPanelWidget* GetLayerContainer(EUILayer Layer) const;

	// --- Bound layer containers. Names must match the WBP. Optional so a WBP may omit layers. ---

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_GameHUD;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Menu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Popup;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Modal;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Tooltip;

private:
	/** Per-layer stacks of pushed widgets (index 0 = bottom, last = top). */
	TMap<EUILayer, TArray<TObjectPtr<UUserWidget>>> LayerStacks;

	/** Layers ordered top to bottom, for back-action resolution. */
	static const TArray<EUILayer>& GetLayersTopDown();
};
