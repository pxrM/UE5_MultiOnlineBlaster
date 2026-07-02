// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "UILayerTypes.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;
class UUIRootWidget;
class UUILayerSubsystem;

/**
 * High-level UI policy: opens and closes UI by GameplayTag using the registry and
 * settings, then drives the low-level UUILayerSubsystem to actually place widgets.
 * Callers reference UI by key and never touch concrete classes or layers.
 *
 *   UUIManagerSubsystem::Get(this)->OpenUI(TAG_UI_Inventory);
 *
 * See Docs/DESIGN.md sections 3-4.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Convenience accessor from any object with a world. Returns nullptr if unavailable. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager", meta = (WorldContext = "WorldContextObject"))
	static UUIManagerSubsystem* Get(const UObject* WorldContextObject);

	/** Create the root from UUISettings::DefaultRootClass if not already present. Returns the root. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	UUIRootWidget* EnsureRoot();

	/**
	 * Open a registered UI by key: look it up in the registry, load the class, ensure
	 * the root exists, and push onto the entry's layer. Returns the widget (or the
	 * already-open instance when duplicates are disallowed).
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	UUserWidget* OpenUI(FGameplayTag Key);

	/** Close a UI previously opened with OpenUI(Key). Returns true if one was closed. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool CloseUI(FGameplayTag Key);

private:
	/** Resolve the layer mechanism subsystem. */
	UUILayerSubsystem* GetLayers() const;

	/** Widgets opened via OpenUI, keyed by tag, so CloseUI can find them and OpenUI can dedupe. */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUserWidget>> OpenWidgets;

	/** Layer each open key landed on, so CloseUI removes from the right stack. */
	TMap<FGameplayTag, EUILayer> OpenLayers;
};
