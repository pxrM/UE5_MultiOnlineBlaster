// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UILayerTypes.h"
#include "UILifecycleTypes.h"
#include "UIWidgetRegistry.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;
class UUIRootWidget;
class UUILayerSubsystem;
class UUIWidgetCache;
class UWorld;
struct FStreamableHandle;

/** Fired when OpenUIAsync finishes. Widget is null on failure. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FUIWidgetOpened, UUserWidget*, Widget);

/** All live instances opened for one registry key (normally one, unless allowed). */
USTRUCT()
struct FUIOpenWidgetList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> Widgets;
};

/** Immutable registry snapshot for one live widget's open cycle. */
USTRUCT()
struct FUIOpenWidgetRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Key;

	UPROPERTY()
	FUIWidgetEntry Entry;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FUIManagerWidgetOpening, UUserWidget*, const FUIOpenContext&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FUIManagerWidgetOpened, UUserWidget*, const FUIOpenContext&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FUIManagerWidgetClosing, UUserWidget*, const FUICloseContext&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FUIManagerWidgetClosed, UUserWidget*, const FUICloseContext&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FUIManagerWidgetActivationChanged, UUserWidget*, FGameplayTag, bool);

/** One caller waiting for a shared asynchronous class load. */
USTRUCT()
struct FUIPendingOpenCall
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UObject> Payload = nullptr;

	UPROPERTY()
	FUIWidgetOpened Callback;
};

USTRUCT()
struct FUIPendingOpenCallList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FUIPendingOpenCall> Calls;
};

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
	FUIManagerWidgetOpening& OnWidgetOpening() { return WidgetOpeningEvent; }
	FUIManagerWidgetOpened& OnWidgetOpened() { return WidgetOpenedEvent; }
	FUIManagerWidgetClosing& OnWidgetClosing() { return WidgetClosingEvent; }
	FUIManagerWidgetClosed& OnWidgetClosed() { return WidgetClosedEvent; }
	FUIManagerWidgetActivationChanged& OnWidgetActivationChanged() { return WidgetActivationChangedEvent; }

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
	UUserWidget* OpenUI(FGameplayTag Key, UObject* Payload = nullptr);

	/**
	 * Like OpenUI, but streams the widget class asynchronously (no frame hitch) and
	 * fires OnOpened when it is pushed. If the UI is already open, OnOpened is invoked
	 * immediately with the existing instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void OpenUIAsync(FGameplayTag Key, FUIWidgetOpened OnOpened, UObject* Payload = nullptr);

	/** Cancel a pending asynchronous open. Waiting callbacks receive null. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool CancelOpenUI(FGameplayTag Key);

	/** Close a UI previously opened with OpenUI(Key). Returns true if one was closed. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool CloseUI(FGameplayTag Key, UObject* Result = nullptr);

	/**
	 * Close a tracked widget instance through the manager. Widget-owned close buttons
	 * should use this so manager tracking and instance caching stay consistent.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool CloseWidget(UUserWidget* Widget, UObject* Result = nullptr);

	/** Close every widget opened through this manager. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void CloseAllUI();

	/** Close all current UI while rejecting lifecycle callbacks that try to reopen. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void ForceCloseAllUI();

	/** Pop the overall top frame. Removal notification keeps manager/cache in sync. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool HandleBackAction();

	/** True when at least one instance is open for the key. */
	UFUNCTION(BlueprintPure, Category = "UI|Manager")
	bool IsUIOpen(FGameplayTag Key) const;

	/** Number of currently open instances for the key. */
	UFUNCTION(BlueprintPure, Category = "UI|Manager")
	int32 GetOpenCount(FGameplayTag Key) const;

	/** Validate the configured Registry and CoverageConfig. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	bool ValidateConfiguration(UPARAM(ref) TArray<FString>& OutErrors) const;

	/** Add or override one in-memory entry. Runtime entries take priority over the DataAsset registry. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager|Runtime Registry")
	bool RegisterRuntimeEntry(
		FGameplayTag Key,
		const FUIWidgetEntry& Entry,
		bool bReplaceExisting,
		FString& OutError);

	/** Remove one runtime override. The DataAsset entry, if present, becomes visible again. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager|Runtime Registry")
	bool UnregisterRuntimeEntry(FGameplayTag Key);

	/** Remove every runtime override without affecting currently open widgets. */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager|Runtime Registry")
	void ClearRuntimeEntries();

	UFUNCTION(BlueprintPure, Category = "UI|Manager|Runtime Registry")
	int32 GetRuntimeEntryCount() const { return RuntimeEntries.Num(); }

	/** Drop all cached classes and cached instances (call on level transitions). */
	UFUNCTION(BlueprintCallable, Category = "UI|Manager")
	void ClearCache();

	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** Resolve the layer mechanism subsystem. */
	UUILayerSubsystem* GetLayers() const;

	/** Streamable-load completion handler for all coalesced requests of one key. */
	void HandleAsyncLoaded(FGameplayTag Key);

	/** Shared push + tracking used by both OpenUI and the async path. */
	UUserWidget* PushEntry(FGameplayTag Key, UObject* Payload);

	/** Runtime overlays win; the configured DataAsset remains the fallback. */
	const FUIWidgetEntry* ResolveEntry(FGameplayTag Key) const;

	/** Most recently opened live instance for a key, or null. */
	UUserWidget* GetLatestOpenWidget(FGameplayTag Key) const;

	/** Remove, untrack, and cache one exact instance. */
	bool CloseTrackedWidget(
		FGameplayTag Key,
		UUserWidget* Widget,
		UObject* Result = nullptr,
		EUIWidgetCloseReason Reason = EUIWidgetCloseReason::Requested);

	bool FindTrackedKey(const UUserWidget* Widget, FGameplayTag& OutKey) const;

	void HandleLayerWidgetRemoving(UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason);
	void HandleLayerWidgetRemoved(UUserWidget* Widget, EUILayer Layer);
	void HandleLayerWidgetActivationChanged(UUserWidget* Widget, EUILayer Layer, bool bIsActive);
	void HandleMapChanged(UWorld* LoadedWorld);
	void CloseAllUIWithReason(EUIWidgetCloseReason Reason);

	/** Widgets opened via OpenUI, grouped by tag to support optional multiple instances. */
	UPROPERTY()
	TMap<FGameplayTag, FUIOpenWidgetList> OpenWidgets;

	/** Per-instance open-time policy; authoritative even if the registry changes. */
	UPROPERTY()
	TMap<TObjectPtr<UUserWidget>, FUIOpenWidgetRecord> OpenRecords;

	/** Script/config entries for this game instance; never serialized into assets. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FUIWidgetEntry> RuntimeEntries;

	/** Close context created by Removing and consumed by Removed. */
	TMap<TObjectPtr<UUserWidget>, FUICloseContext> ClosingContexts;

	/** Owns class + instance caching (see UUIWidgetCache). */
	UPROPERTY()
	TObjectPtr<UUIWidgetCache> Cache;

	/** One streamable handle per key; repeated requests share the same disk load. */
	TMap<FGameplayTag, TSharedPtr<FStreamableHandle>> PendingLoadHandles;

	/** Caller payloads and callbacks retained while a shared load is pending. */
	UPROPERTY()
	TMap<FGameplayTag, FUIPendingOpenCallList> PendingOpenCalls;

	FDelegateHandle MapChangedHandle;
	FUIManagerWidgetOpening WidgetOpeningEvent;
	FUIManagerWidgetOpened WidgetOpenedEvent;
	FUIManagerWidgetClosing WidgetClosingEvent;
	FUIManagerWidgetClosed WidgetClosedEvent;
	FUIManagerWidgetActivationChanged WidgetActivationChangedEvent;
	bool bRejectNewOpens = false;
	bool bIsShuttingDown = false;
};
