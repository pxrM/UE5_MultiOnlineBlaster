// Copyright TikiStar. All Rights Reserved.

#include "UIWidgetCache.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectGlobals.h"

bool UUIWidgetCache::CachesClass(EUICachePolicy Policy)
{
	return Policy != EUICachePolicy::Transient;
}

bool UUIWidgetCache::CachesInstance(EUICachePolicy Policy)
{
	return Policy == EUICachePolicy::KeepUntilIdle
		|| Policy == EUICachePolicy::KeepUntilSceneChange
		|| Policy == EUICachePolicy::KeepPersistent;
}

void UUIWidgetCache::Initialize()
{
	// Real-time ticker (world-independent) so idle reaping survives level loads.
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UUIWidgetCache::Tick), ReapInterval);

	MapChangedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UUIWidgetCache::HandleMapChanged);
}

void UUIWidgetCache::Shutdown()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	if (MapChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapChangedHandle);
		MapChangedHandle.Reset();
	}
	ClearAll();
}

TSubclassOf<UUserWidget> UUIWidgetCache::ResolveClass(FGameplayTag Key, const FUIWidgetEntry& Entry)
{
	if (TSubclassOf<UUserWidget>* Cached = Classes.Find(Key))
	{
		return *Cached;
	}

	TSubclassOf<UUserWidget> WidgetClass = Entry.WidgetClass.LoadSynchronous();
	if (WidgetClass && CachesClass(Entry.CachePolicy))
	{
		Classes.Add(Key, WidgetClass);
	}
	return WidgetClass;
}

UUserWidget* UUIWidgetCache::TakeInstance(FGameplayTag Key)
{
	if (FUICachedInstance* Cached = ClosedInstances.Find(Key))
	{
		UUserWidget* Widget = Cached->Widget;
		ClosedInstances.Remove(Key);
		return Widget;
	}
	return nullptr;
}

void UUIWidgetCache::ReturnInstance(FGameplayTag Key, UUserWidget* Widget, const FUIWidgetEntry& Entry)
{
	if (!Widget || !CachesInstance(Entry.CachePolicy))
	{
		// Not cached: the manager already removed it, so it becomes unreferenced and is collected.
		return;
	}

	Widget->SetVisibility(ESlateVisibility::Collapsed);

	FUICachedInstance Cached;
	Cached.Widget = Widget;
	Cached.Policy = Entry.CachePolicy;
	Cached.IdleTimeout = Entry.IdleTimeoutSeconds;
	Cached.ClosedTime = FPlatformTime::Seconds();
	ClosedInstances.Add(Key, Cached);
}

void UUIWidgetCache::ClearAll()
{
	Classes.Empty();
	ClosedInstances.Empty();
}

bool UUIWidgetCache::Tick(float /*DeltaTime*/)
{
	ReapIdle();
	return true; // keep ticking
}

void UUIWidgetCache::ReapIdle()
{
	if (ClosedInstances.Num() == 0)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	for (auto It = ClosedInstances.CreateIterator(); It; ++It)
	{
		const FUICachedInstance& Cached = It.Value();
		if (!Cached.Widget)
		{
			It.RemoveCurrent();
			continue;
		}
		if (Cached.Policy == EUICachePolicy::KeepUntilIdle
			&& (Now - Cached.ClosedTime) >= Cached.IdleTimeout)
		{
			// Drop the hard ref; the widget is collected on the next GC.
			It.RemoveCurrent();
		}
	}
}

void UUIWidgetCache::HandleMapChanged(UWorld* /*World*/)
{
	// A new scene loaded: free everything except session-persistent instances.
	for (auto It = ClosedInstances.CreateIterator(); It; ++It)
	{
		if (It.Value().Policy != EUICachePolicy::KeepPersistent)
		{
			It.RemoveCurrent();
		}
	}
}
