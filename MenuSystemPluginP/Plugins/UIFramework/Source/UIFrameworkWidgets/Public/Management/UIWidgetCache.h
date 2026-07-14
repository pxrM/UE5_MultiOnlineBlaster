// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "GameplayTagContainer.h"
#include "Containers/Ticker.h"
#include "UIWidgetRegistry.h"
#include "UIWidgetCache.generated.h"

class UUserWidget;
class UWorld;

/** A cached, currently-closed widget instance plus the metadata driving its lifetime. */
USTRUCT()
struct FUICachedInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	EUICachePolicy Policy = EUICachePolicy::Transient;

	/** Idle seconds before destruction (KeepUntilIdle only). */
	float IdleTimeout = 0.f;

	/** Real time (seconds) the instance was returned to the cache. */
	double ClosedTime = 0.0;
};

/**
 * Owns all UI caching so the manager stays about open/close policy, not storage.
 * Two independent caches:
 *   - class cache: hard-refs loaded widget classes so they never re-load from disk.
 *   - instance cache: keeps closed widget instances alive for reuse (preserving state),
 *     freed per policy — idle timeout (TTL), scene change, or session end.
 * See Docs/DESIGN.md sections 3-4 & 7.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUIWidgetCache : public UObject
{
	GENERATED_BODY()

public:
	/** Start the idle reaper and subscribe to scene changes. */
	void Initialize();

	/** Stop the reaper, unsubscribe, and drop everything. */
	void Shutdown();

	/** Resolve a key's widget class, caching it (hard ref) unless the policy is Transient. */
	TSubclassOf<UUserWidget> ResolveClass(FGameplayTag Key, const FUIWidgetEntry& Entry);

	/** Take a cached closed instance for reuse (removes it from the cache), or null. */
	UUserWidget* TakeInstance(FGameplayTag Key);

	/** Return a closed instance to the cache if its policy keeps instances; else a no-op. */
	void ReturnInstance(FGameplayTag Key, UUserWidget* Widget, const FUIWidgetEntry& Entry);

	/** Drop all cached classes and instances. */
	void ClearAll();

	/** True if the policy keeps the widget class resident. */
	static bool CachesClass(EUICachePolicy Policy);

	/** True if the policy keeps the widget instance alive after close. */
	static bool CachesInstance(EUICachePolicy Policy);

private:
	bool Tick(float DeltaTime);
	void ReapIdle();
	void HandleMapChanged(UWorld* World);

	/** Key -> resident widget class. */
	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> Classes;

	/** Key -> closed, reusable instance. */
	UPROPERTY()
	TMap<FGameplayTag, FUICachedInstance> ClosedInstances;

	FTSTicker::FDelegateHandle TickHandle;
	FDelegateHandle MapChangedHandle;

	/** Reaper cadence (seconds). */
	float ReapInterval = 5.f;
};
