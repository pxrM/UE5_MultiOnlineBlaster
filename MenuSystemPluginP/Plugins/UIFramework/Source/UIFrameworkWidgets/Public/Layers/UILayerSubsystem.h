// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIRootWidget.h"
#include "UILayerSubsystem.generated.h"

class APlayerController;

/** Fired for every widget removed through any layer-stack path. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FUILayerWidgetRemoved, UUserWidget*, EUILayer);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUILayerWidgetRemoving, UUserWidget*, EUILayer, EUIWidgetCloseReason);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUILayerWidgetActivationChanged, UUserWidget*, EUILayer, bool);

/**
 * Low-level layer mechanism: owns the single UI root widget and forwards push/pop
 * to its layer stacks. Pure mechanism — it knows nothing about the widget registry,
 * GameplayTags, or settings. High-level "open UI by key" policy lives in
 * UUIManagerSubsystem, which drives this. See Docs/DESIGN.md section 4.
 *
 * One root per game instance (single viewport). For split-screen, promote this
 * to a ULocalPlayerSubsystem — the layer logic itself is unchanged.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUILayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Native lifecycle event consumed by the high-level manager. */
	FUILayerWidgetRemoved& OnWidgetRemoved() { return WidgetRemovedEvent; }
	FUILayerWidgetRemoving& OnWidgetRemoving() { return WidgetRemovingEvent; }
	FUILayerWidgetActivationChanged& OnWidgetActivationChanged() { return WidgetActivationChangedEvent; }

	/** Convenience accessor from any object with a world. Returns nullptr if unavailable. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer", meta = (WorldContext = "WorldContextObject"))
	static UUILayerSubsystem* Get(const UObject* WorldContextObject);

	/**
	 * Create the root widget, add it to the viewport, and keep it. Tears down any
	 * existing root first. Pass the local player controller when CommonUI input
	 * routing is required. Returns the created root (nullptr on failure).
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUIRootWidget* InitializeRoot(TSubclassOf<UUIRootWidget> RootClass, APlayerController* OwningPlayer = nullptr, int32 ZOrder = 0);

	/** Remove the root from the viewport and drop it. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void TeardownRoot();

	/** The current root, or nullptr if not initialized. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUIRootWidget* GetRoot() const { return Root; }

	// --- Layer forwarders. Safe no-ops / warnings if no root. ---

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass);

	/** Push an existing widget instance onto a layer (for pooled / cached reuse). */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushWidget(EUILayer Layer, UUserWidget* Widget);

	UUserWidget* PushWidgetDeferred(EUILayer Layer, UUserWidget* Widget, bool bBlocksInput = true, bool bHandlesBack = true);
	void RefreshStack();

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool PopFromLayer(EUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void PopAllFromLayer(EUILayer Layer);

	/** Remove a specific widget from a layer (not necessarily the top). Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool RemoveWidget(EUILayer Layer, UUserWidget* Widget);
	bool RemoveWidgetWithReason(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason);

	bool IsWidgetActive(const UUserWidget* Widget) const;

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	bool HandleBackAction();

	// USubsystem
	virtual void Deinitialize() override;

private:
	void HandleRootWidgetRemoved(UUserWidget* Widget, EUILayer Layer);
	void HandleRootWidgetRemoving(UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason);
	void HandleRootWidgetActivationChanged(UUserWidget* Widget, EUILayer Layer, bool bIsActive);
	void TeardownRootWithReason(EUIWidgetCloseReason Reason);

	UPROPERTY()
	TObjectPtr<UUIRootWidget> Root;

	FUILayerWidgetRemoved WidgetRemovedEvent;
	FUILayerWidgetRemoving WidgetRemovingEvent;
	FUILayerWidgetActivationChanged WidgetActivationChangedEvent;
};
