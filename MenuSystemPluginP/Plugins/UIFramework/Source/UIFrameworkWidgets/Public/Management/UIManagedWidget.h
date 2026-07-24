// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UILifecycleTypes.h"
#include "UIManagedWidget.generated.h"

UINTERFACE(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIManagedWidget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional lifecycle contract for widgets opened through UUIManagerSubsystem.
 * Low-level widgets pushed directly through UUILayerSubsystem are intentionally
 * outside this protocol.
 */
class UIFRAMEWORKWIDGETS_API IUIManagedWidget
{
	GENERATED_BODY()

public:
	/** After construction/attachment, before Opened and before stack activation. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIOpening(const FUIOpenContext& Context);
	virtual void OnUIOpening_Implementation(const FUIOpenContext& Context) {}

	/** After the widget is attached and tracked, before activation notification. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIOpened(const FUIOpenContext& Context);
	virtual void OnUIOpened_Implementation(const FUIOpenContext& Context) {}

	/** When this widget becomes the active top frame. May happen repeatedly. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIActivated();
	virtual void OnUIActivated_Implementation() {}

	/** When this widget stops being the active top frame. May happen repeatedly. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIDeactivated();
	virtual void OnUIDeactivated_Implementation() {}

	/** Before detaching from the layer. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIClosing(const FUICloseContext& Context);
	virtual void OnUIClosing_Implementation(const FUICloseContext& Context) {}

	/** After detaching from the layer, before an optional return to cache. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIClosed(const FUICloseContext& Context);
	virtual void OnUIClosed_Implementation(const FUICloseContext& Context) {}

	/** Immediately after an instance is taken from the closed-instance cache. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Lifecycle")
	void OnUIRestoredFromCache(const FUIOpenContext& Context);
	virtual void OnUIRestoredFromCache_Implementation(const FUIOpenContext& Context) {}
};
