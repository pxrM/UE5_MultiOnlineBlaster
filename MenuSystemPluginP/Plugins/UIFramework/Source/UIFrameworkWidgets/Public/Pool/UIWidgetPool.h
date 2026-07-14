// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "Engine/TimerHandle.h"
#include "UIWidgetPool.generated.h"

class UUserWidget;
class UWorld;

/** One idle instance plus the time (world seconds) it was returned, for TTL reaping. */
USTRUCT()
struct FUIPooledWidget
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	float FreeTime = 0.f;
};

/** Free-list bucket for one widget class (USTRUCT so it can be a TMap value in a UPROPERTY). */
USTRUCT()
struct FUIWidgetPoolBucket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FUIPooledWidget> Entries;
};

/**
 * Recycles UUserWidget instances by class to avoid Create/Destroy churn for
 * high-frequency UI (damage numbers, toasts, non-UListView item streams).
 *
 * Acquire() reuses a hidden free instance or creates one and tracks it as active;
 * Release() resets and returns it to the free list (capped by MaxFreePerClass).
 * Idle instances unused for IdleReapSeconds are reaped by a periodic timer, so the
 * pool shrinks on its own after a burst without a full-order LRU. Widgets
 * implementing IUIPoolableWidget get OnAcquiredFromPool / OnReturnedToPool
 * callbacks. See Docs/DESIGN.md section 7 (performance).
 *
 * Lives in the Widgets module (not Core) because it pools UUserWidget, a UMG type.
 * For virtualized rows prefer UListView (UUIListBase), which pools internally.
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIWidgetPool : public UObject
{
	GENERATED_BODY()

public:
	/** Max idle instances kept per class. Extra releases are dropped to GC. 0 = unbounded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Pool")
	int32 MaxFreePerClass = 32;

	/** Idle instances unused this long (world seconds) are reaped. <= 0 disables TTL reaping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Pool")
	float IdleReapSeconds = 30.f;

	/** How often the reaper checks for expired idle instances (world seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Pool")
	float ReapInterval = 5.f;

	/** Set the world used to create widgets and drive the reaper. Call once before Acquire. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void SetWorld(UWorld* InWorld);

	/** Get a widget of the given class: a recycled one if available, else a new one. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	UUserWidget* Acquire(TSubclassOf<UUserWidget> WidgetClass);

	/** Reset a widget and return it to its class's free list for reuse. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void Release(UUserWidget* Widget);

	/** Release every currently active widget. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void ReleaseAll();

	/** Pre-create Count instances of a class so the first Acquire calls are cheap. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void Prewarm(TSubclassOf<UUserWidget> WidgetClass, int32 Count);

	/** Drop idle instances of a class down to KeepCount (frees the rest to GC). */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void TrimClass(TSubclassOf<UUserWidget> WidgetClass, int32 KeepCount);

	/** Drop all idle instances (active widgets are untouched). */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void Reset();

	/** Remove idle instances older than IdleReapSeconds now (also runs on a timer). */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	void ReapExpired();

	/** Idle (reusable) instance count for a class. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	int32 GetNumFree(TSubclassOf<UUserWidget> WidgetClass) const;

	/** Number of widgets currently checked out via Acquire. */
	UFUNCTION(BlueprintCallable, Category = "UI|Pool")
	int32 GetNumActive() const { return ActiveWidgets.Num(); }

	// UObject
	virtual void BeginDestroy() override;

private:
	UUserWidget* CreateForClass(TSubclassOf<UUserWidget> WidgetClass);
	static void NotifyAcquired(UUserWidget* Widget);
	static void NotifyReturned(UUserWidget* Widget);
	float GetNow() const;
	void StartReaper();
	void StopReaper();

	UPROPERTY()
	TWeakObjectPtr<UWorld> World;

	/** Class -> list of hidden, reusable instances. */
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, FUIWidgetPoolBucket> FreeByClass;

	/** Widgets currently handed out (kept alive + enables ReleaseAll / stats). */
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> ActiveWidgets;

	FTimerHandle ReapTimerHandle;
};
