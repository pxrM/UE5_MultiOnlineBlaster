// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayerTypes.generated.h"

/**
 * UI layers ordered bottom to top. Lower value = drawn first (further back).
 * Values are spaced so new layers can be inserted without renumbering.
 *
 * Lives in Core (no UMG dependency) so pure logic — event bus, view models,
 * config — can reference layers without pulling in the widget module.
 */
UENUM(BlueprintType)
enum class EUILayer : uint8
{
	GameHUD = 0		UMETA(DisplayName = "Game HUD"),
	Menu    = 10	UMETA(DisplayName = "Menu"),
	Popup   = 20	UMETA(DisplayName = "Popup"),
	Modal   = 30	UMETA(DisplayName = "Modal"),
	Tooltip = 40	UMETA(DisplayName = "Tooltip")
};
