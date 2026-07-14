// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UISettings.generated.h"

class UUIRootWidget;
class UUIWidgetRegistry;

/**
 * Project-level UI framework config, editable under Project Settings > Game > UI Framework.
 * A new project reuses the framework by pointing these at its own Root WBP and registry.
 * See Docs/DESIGN.md sections 3-4.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "UI Framework"))
class UIFRAMEWORKWIDGETS_API UUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Root WBP created and added to the viewport (must derive from UUIRootWidget). */
	UPROPERTY(config, EditAnywhere, Category = "UI Framework")
	TSoftClassPtr<UUIRootWidget> DefaultRootClass;

	/** Registry mapping GameplayTag keys to widget entries. */
	UPROPERTY(config, EditAnywhere, Category = "UI Framework")
	TSoftObjectPtr<UUIWidgetRegistry> Registry;

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
