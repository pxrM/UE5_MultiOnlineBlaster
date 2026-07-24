// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UILayerTypes.h"
#include "UILifecycleTypes.h"
#include "UIRootWidget.generated.h"

class UPanelWidget;
class UUILayerStack;

/** Root-level removal event forwarded from the internal stack. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FUIRootWidgetRemoved, UUserWidget*, EUILayer);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUIRootWidgetRemoving, UUserWidget*, EUILayer, EUIWidgetCloseReason);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUIRootWidgetActivationChanged, UUserWidget*, EUILayer, bool);

/**
 * Root widget for the whole UI. One instance lives in the viewport.
 *
 * This class is the VIEW: it owns the bound per-layer container panels and forwards
 * all push/pop/visibility policy to a UUILayerStack. See Docs/DESIGN.md section 4.
 *
 * WBP usage: reparent the root WBP to this class, then name the per-layer containers
 * to match the BindWidget properties below (Layer_PersistentSystem through
 * Layer_Tips). EUILayer::Max has no panel.
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Native removal notification; used by UUILayerSubsystem. */
	FUIRootWidgetRemoved& OnWidgetRemoved() { return WidgetRemovedEvent; }
	FUIRootWidgetRemoving& OnWidgetRemoving() { return WidgetRemovingEvent; }
	FUIRootWidgetActivationChanged& OnWidgetActivationChanged() { return WidgetActivationChangedEvent; }

	/** Create a widget of the class and push it onto the layer. Returns it. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass);

	/** Push an existing widget instance onto a layer (for pooled / cached reuse). */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushWidget(EUILayer Layer, UUserWidget* Widget);

	/** Manager-only path: attach now, defer visibility/input recompute until lifecycle Opened. */
	UUserWidget* PushWidgetDeferred(EUILayer Layer, UUserWidget* Widget, bool bBlocksInput = true, bool bHandlesBack = true);
	void RefreshStack();

	/** Pop the top widget off the given layer. Returns true if a widget was removed. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool PopFromLayer(EUILayer Layer);

	/** Remove every widget on the given layer. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void PopAllFromLayer(EUILayer Layer);

	/** Remove a specific widget from a layer's stack. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool RemoveWidget(EUILayer Layer, UUserWidget* Widget);
	bool RemoveWidgetWithReason(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason);

	/** Top widget of a layer, or nullptr if the layer is empty. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* GetTopWidget(EUILayer Layer) const;

	/** Top widget across every layer, or nullptr if the stack is empty. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* GetTopMostWidget() const;

	/** True if the layer has at least one widget. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool IsLayerActive(EUILayer Layer) const;
	bool IsWidgetActive(const UUserWidget* Widget) const;

	/**
	 * Handle a back / cancel action: pop the single top-most widget across all layers.
	 * Returns true if something was consumed.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool HandleBackAction();

	/** Remove every managed widget. Used when the root is torn down. */
	void ClearAllWidgets(EUIWidgetCloseReason Reason = EUIWidgetCloseReason::RootTeardown);

protected:
	virtual void NativeOnInitialized() override;

	/** Resolve the bound container panel for a layer, or nullptr if not bound. */
	UPanelWidget* GetLayerContainer(EUILayer Layer) const;

	// --- Bound layer containers. Names must match the WBP. Optional so a WBP may omit layers. ---

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_PersistentSystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Loading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Background;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Dock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_FullWindow;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_PopupWindow;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Guide;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Notification;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Layer")
	TObjectPtr<UPanelWidget> Layer_Tips;

private:
	void HandleStackWidgetRemoved(UUserWidget* Widget, EUILayer Layer);
	void HandleStackWidgetRemoving(UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason);
	void HandleStackWidgetActivationChanged(UUserWidget* Widget, EUILayer Layer, bool bIsActive);

	/** Owns frame ordering + show/hide policy (see UUILayerStack). */
	UPROPERTY()
	TObjectPtr<UUILayerStack> Stack;

	FUIRootWidgetRemoved WidgetRemovedEvent;
	FUIRootWidgetRemoving WidgetRemovingEvent;
	FUIRootWidgetActivationChanged WidgetActivationChangedEvent;
};
