// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIPoolableWidget.generated.h"

UINTERFACE(BlueprintType, MinimalAPI)
class UUIPoolableWidget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional interface for widgets managed by UUIWidgetPool. A recycled widget keeps
 * its old state, so implement these to reset visuals on reuse and stop work on return.
 * Widgets that don't implement it are still pooled — they just get no callbacks.
 */
class IUIPoolableWidget
{
	GENERATED_BODY()

public:
	/** Called right before the pool hands this widget out. Reset it to a clean state. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Pool")
	void OnAcquiredFromPool();

	/** Called when the widget is returned to the pool. Stop timers / animations here. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Pool")
	void OnReturnedToPool();
};
