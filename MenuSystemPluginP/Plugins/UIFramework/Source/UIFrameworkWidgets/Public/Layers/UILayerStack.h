// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/SlateWrapperTypes.h"
#include "UILayerTypes.h"
#include "UILifecycleTypes.h"
#include "UILayerStack.generated.h"

class UUserWidget;
class UPanelWidget;
class UUICoverageConfig;

/** Fired after a frame is removed from the stack and detached from its panel. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FUIStackWidgetRemoved, UUserWidget*, EUILayer);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUIStackWidgetRemoving, UUserWidget*, EUILayer, EUIWidgetCloseReason);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUIStackWidgetActivationChanged, UUserWidget*, EUILayer, bool);

/**
 * One frame in the UI stack: the widget and the layer it belongs to (which doubles
 * as its priority tier). Whether it hides frames below is decided by the coverage
 * matrix (UUICoverageConfig), not stored per-frame.
 */
USTRUCT()
struct FUIStackFrame
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	EUILayer Layer = EUILayer::FullWindow;

	UPROPERTY()
	bool bBlocksInput = true;

	UPROPERTY()
	bool bHandlesBack = true;

	/** Visibility captured before coverage collapses this frame. */
	UPROPERTY()
	ESlateVisibility VisibilityBeforeCoverage = ESlateVisibility::Visible;

	/** True only while CheckShow owns the widget's Collapsed state. */
	bool bHiddenByCoverage = false;

	bool bIsActive = false;
	bool bIsClosing = false;
};

/**
 * Owns the ordered UI frames and all show/hide policy, so UIRootWidget stays about
 * its bound container panels (the view) and delegates behavior here.
 *
 * Modeled on the single-stack UIStack pattern (see Docs/DESIGN.md): frames live in
 * ONE array ordered bottom->top, with EUILayer as the priority tier (higher layer =
 * higher tier, same tier = FILO). CheckShow runs a two-phase pass (mark top-down,
 * apply bottom-up) so configured covering layers collapse matching frames below
 * them, including across layers. Collapsed frames don't render or hit-test.
 *
 * Not adopted from the Lua framework: slap FIFO frames and overflow eviction
 * (UUIWidgetCache covers lifetime). Layered Z is still enforced by the WBP
 * containers; this class decides visibility and activation within that ordering.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUILayerStack : public UObject
{
	GENERATED_BODY()

public:
	/** Native lifecycle notification used by the layer subsystem and manager. */
	FUIStackWidgetRemoved& OnWidgetRemoved() { return WidgetRemovedEvent; }
	FUIStackWidgetRemoving& OnWidgetRemoving() { return WidgetRemovingEvent; }
	FUIStackWidgetActivationChanged& OnWidgetActivationChanged() { return WidgetActivationChangedEvent; }

	/** Set the coverage matrix that decides cross-layer hiding. Null = no hiding. */
	void SetCoverageConfig(const UUICoverageConfig* InConfig);

	/** Add a widget to a layer's container and push a frame; recomputes visibility. Returns the widget. */
	UUserWidget* Push(UPanelWidget* Container, EUILayer Layer, UUserWidget* Widget, bool bRefresh = true, bool bBlocksInput = true, bool bHandlesBack = true);

	/** Recompute visibility and activation after a deferred managed push. */
	void Refresh() { CheckShow(); }

	/** Pop the top-most frame of a layer. Returns true if one was removed. */
	bool PopTop(EUILayer Layer, EUIWidgetCloseReason Reason = EUIWidgetCloseReason::LayerPop);

	/** Remove a specific widget from the expected layer. Returns true if found. */
	bool Remove(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason = EUIWidgetCloseReason::Requested);

	/** Remove every frame of a layer. */
	void ClearLayer(EUILayer Layer, EUIWidgetCloseReason Reason = EUIWidgetCloseReason::LayerClear);

	/** Remove every frame from every layer. */
	void ClearAll(EUIWidgetCloseReason Reason = EUIWidgetCloseReason::RootTeardown);

	/** Pop the single top-most frame across all layers (back / cancel). Returns true if consumed. */
	bool PopTopMost(EUIWidgetCloseReason Reason = EUIWidgetCloseReason::Back);

	/** Top widget of a layer, or nullptr. */
	UUserWidget* GetTop(EUILayer Layer) const;

	/** Top widget across all layers, or nullptr. */
	UUserWidget* GetTopMost() const;

	/** True if a layer has at least one frame. */
	bool IsLayerActive(EUILayer Layer) const;

	/** True when this frame is the currently active top. */
	bool IsWidgetActive(const UUserWidget* Widget) const;

	/** Total live frame count. */
	int32 Num() const { return Frames.Num(); }

private:
	/** Detach and remove one frame, broadcasting the lifecycle notification. */
	bool RemoveFrameAt(int32 Index, EUIWidgetCloseReason Reason);

	/** Two-phase visibility + activation recompute across all frames. */
	void CheckShow();
	void CheckShowOnce();

	/** Insertion index keeping frames ordered bottom->top by layer tier (same tier = on top). */
	int32 FindInsertIndex(EUILayer Layer) const;

	/** Frames ordered bottom (index 0) to top (last). */
	UPROPERTY()
	TArray<FUIStackFrame> Frames;

	/** Coverage matrix driving CheckShow. Weak: owned by settings, may be null. */
	UPROPERTY()
	TObjectPtr<const UUICoverageConfig> CoverageConfig;

	FUIStackWidgetRemoved WidgetRemovedEvent;
	FUIStackWidgetRemoving WidgetRemovingEvent;
	FUIStackWidgetActivationChanged WidgetActivationChangedEvent;

	/** Lifecycle callbacks may mutate the stack while it is being refreshed. */
	bool bIsRefreshing = false;
	bool bRefreshRequested = false;
};
