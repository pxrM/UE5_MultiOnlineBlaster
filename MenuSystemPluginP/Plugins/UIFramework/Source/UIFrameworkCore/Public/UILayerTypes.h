// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayerTypes.generated.h"

/**
 * UI layers ordered bottom to top. Lower value = drawn first (further back).
 * Max is a sentinel and must never be used as a real layer.
 *
 * Lives in Core (no UMG dependency) so pure logic — event bus, view models,
 * config — can reference layers without pulling in the widget module.
 */
UENUM(BlueprintType)
enum class EUILayer : uint8
{
	PersistentSystem = 0 UMETA(DisplayName = "Persistent System"),
	Loading              UMETA(DisplayName = "Loading"),
	Background           UMETA(DisplayName = "Background"),
	Dock                 UMETA(DisplayName = "Dock"),
	FullWindow           UMETA(DisplayName = "Full Window"),
	PopupWindow          UMETA(DisplayName = "Popup Window"),
	Guide                UMETA(DisplayName = "Guide"),
	Notification         UMETA(DisplayName = "Notification"),
	Tips                 UMETA(DisplayName = "Tips"),
	Max                  UMETA(Hidden)
};

constexpr bool IsValidUILayer(EUILayer Layer)
{
	return Layer < EUILayer::Max;
}
