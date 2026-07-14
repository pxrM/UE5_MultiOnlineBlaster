// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UILayerTypes.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;
class UUIRootWidget;
class UUILayerSubsystem;
class UUIWidgetCache;

/** Fired when OpenUIAsync finishes. Widget is null on failure. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FUIWidgetOpened, UUserWidget*, Widget);

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

	/**
	 * Like OpenUI, but streams the widget class asynchronously (no frame hitch) and
	 * fires OnOpened when it is pushed. If the UI is already open, OnOpened is invoked
	 * immediately with the existing instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void OpenUIAsync(FGameplayTag Key, FUIWidgetOpened OnOpened);

	/** Close a UI previously opened with OpenUI(Key). Returns true if one was closed. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool CloseUI(FGameplayTag Key);

	/** Drop all cached classes and cached instances (call on level transitions). */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void ClearCache();

	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** Resolve the layer mechanism subsystem. */
	UUILayerSubsystem* GetLayers() const;

	/** Streamable-load completion handler for OpenUIAsync. */
	void HandleAsyncLoaded(FGameplayTag Key, FUIWidgetOpened OnOpened);

	/** Shared push + tracking used by both OpenUI and the async path. */
	UUserWidget* PushEntry(FGameplayTag Key);

	/** Widgets opened via OpenUI, keyed by tag, so CloseUI can find them and OpenUI can dedupe. */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUserWidget>> OpenWidgets;

	/** Layer each open key landed on, so CloseUI removes from the right stack. */
	TMap<FGameplayTag, EUILayer> OpenLayers;

	/** Owns class + instance caching (see UUIWidgetCache). */
	UPROPERTY()
	TObjectPtr<UUIWidgetCache> Cache;
};
