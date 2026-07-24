// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"
#include "Containers/Ticker.h"
#include "UIWidgetRegistry.h"
#include "UIWidgetCache.generated.h"

class UUserWidget;

/** A cached, currently-closed widget instance plus the metadata driving its lifetime. */
USTRUCT()
struct FUICachedInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	FSoftObjectPath WidgetClassPath;

	UPROPERTY()
	EUICachePolicy Policy = EUICachePolicy::Transient;

	/** Idle seconds before destruction (KeepUntilIdle only). */
	float IdleTimeout = 0.f;

	/** Real time (seconds) the instance was returned to the cache. */
	double ClosedTime = 0.0;
};

/** Closed instances for one registry key, newest at the end. */
USTRUCT()
struct FUICachedInstanceList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FUICachedInstance> Instances;
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
	/** Start the idle reaper. Safe to call repeatedly. */
	void Initialize();

	/** Stop the reaper, unsubscribe, and drop everything. */
	void Shutdown();

	/** Resolve a key's widget class, caching it (hard ref) unless the policy is Transient. */
	TSubclassOf<UUserWidget> ResolveClass(FGameplayTag Key, const FUIWidgetEntry& Entry);

	/** Take a cached closed instance for reuse (removes it from the cache), or null. */
	UUserWidget* TakeInstance(FGameplayTag Key, const FUIWidgetEntry& Entry);

	/** Return a closed instance to the cache if its policy keeps instances; else a no-op. */
	void ReturnInstance(FGameplayTag Key, UUserWidget* Widget, const FUIWidgetEntry& Entry);

	/** Drop all cached classes and instances. */
	void ClearAll();

	/** Drop the cached class and all closed instances for one registry key. */
	void Remove(FGameplayTag Key);

	/** Drop scene-scoped instances after the manager has closed the old scene. */
	void HandleSceneChange();

	/** Number of closed reusable instances held for a key. */
	int32 GetNumInstances(FGameplayTag Key) const;

	/** True if the policy keeps the widget class resident. */
	static bool CachesClass(EUICachePolicy Policy);

	/** True if the policy keeps the widget instance alive after close. */
	static bool CachesInstance(EUICachePolicy Policy);

private:
	bool Tick(float DeltaTime);
	void ReapIdle();

	/** Key -> resident widget class. */
	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> Classes;

	UPROPERTY()
	TMap<FGameplayTag, FSoftObjectPath> ClassPaths;

	/** Key -> closed, reusable instances (supports bAllowMultiple). */
	UPROPERTY()
	TMap<FGameplayTag, FUICachedInstanceList> ClosedInstances;

	FTSTicker::FDelegateHandle TickHandle;

	/** Reaper cadence (seconds). */
	float ReapInterval = 5.f;
};
