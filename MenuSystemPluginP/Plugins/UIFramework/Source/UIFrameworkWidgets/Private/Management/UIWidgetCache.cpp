// Copyright TikiStar. All Rights Reserved.

#include "UIWidgetCache.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"

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
	if (TickHandle.IsValid())
	{
		return;
	}

	// Real-time ticker (world-independent) so idle reaping survives level loads.
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UUIWidgetCache::Tick), ReapInterval);

}

void UUIWidgetCache::Shutdown()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	ClearAll();
}

TSubclassOf<UUserWidget> UUIWidgetCache::ResolveClass(FGameplayTag Key, const FUIWidgetEntry& Entry)
{
	const FSoftObjectPath RequestedPath = Entry.WidgetClass.ToSoftObjectPath();
	if (TSubclassOf<UUserWidget>* Cached = Classes.Find(Key))
	{
		if (CachesClass(Entry.CachePolicy))
		{
			if (const FSoftObjectPath* CachedPath = ClassPaths.Find(Key); CachedPath && *CachedPath == RequestedPath)
			{
				return *Cached;
			}
		}
		Classes.Remove(Key);
		ClassPaths.Remove(Key);
	}

	TSubclassOf<UUserWidget> WidgetClass = Entry.WidgetClass.LoadSynchronous();
	if (WidgetClass && CachesClass(Entry.CachePolicy))
	{
		Classes.Add(Key, WidgetClass);
		ClassPaths.Add(Key, RequestedPath);
	}
	return WidgetClass;
}

UUserWidget* UUIWidgetCache::TakeInstance(FGameplayTag Key, const FUIWidgetEntry& Entry)
{
	FUICachedInstanceList* List = ClosedInstances.Find(Key);
	if (!List)
	{
		return nullptr;
	}

	UUserWidget* Widget = nullptr;
	const FSoftObjectPath RequestedPath = Entry.WidgetClass.ToSoftObjectPath();
	while (!List->Instances.IsEmpty() && !Widget)
	{
		const FUICachedInstance Cached = List->Instances.Pop();
		if (IsValid(Cached.Widget) && Cached.WidgetClassPath == RequestedPath)
		{
			Widget = Cached.Widget;
		}
		else
		{
			Widget = nullptr;
		}
	}

	if (List->Instances.IsEmpty())
	{
		ClosedInstances.Remove(Key);
	}
	return Widget;
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
	Cached.WidgetClassPath = Entry.WidgetClass.ToSoftObjectPath();
	Cached.Policy = Entry.CachePolicy;
	Cached.IdleTimeout = Entry.IdleTimeoutSeconds;
	Cached.ClosedTime = FPlatformTime::Seconds();

	FUICachedInstanceList& List = ClosedInstances.FindOrAdd(Key);
	List.Instances.RemoveAll(
		[Widget](const FUICachedInstance& Existing) { return Existing.Widget == Widget; });
	if (Entry.MaxCachedInstances > 0 && List.Instances.Num() >= Entry.MaxCachedInstances)
	{
		// Keep the newest closed instances; the oldest one is least likely to be reused.
		const int32 RemoveCount = List.Instances.Num() - Entry.MaxCachedInstances + 1;
		List.Instances.RemoveAt(0, RemoveCount);
	}
	List.Instances.Add(MoveTemp(Cached));
}

void UUIWidgetCache::ClearAll()
{
	Classes.Empty();
	ClassPaths.Empty();
	ClosedInstances.Empty();
}

void UUIWidgetCache::Remove(FGameplayTag Key)
{
	Classes.Remove(Key);
	ClassPaths.Remove(Key);
	ClosedInstances.Remove(Key);
}

int32 UUIWidgetCache::GetNumInstances(FGameplayTag Key) const
{
	const FUICachedInstanceList* List = ClosedInstances.Find(Key);
	return List ? List->Instances.Num() : 0;
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
		TArray<FUICachedInstance>& Instances = It.Value().Instances;
		for (int32 Index = Instances.Num() - 1; Index >= 0; --Index)
		{
			const FUICachedInstance& Cached = Instances[Index];
			if (!IsValid(Cached.Widget)
				|| (Cached.Policy == EUICachePolicy::KeepUntilIdle
					&& (Now - Cached.ClosedTime) >= Cached.IdleTimeout))
			{
				Instances.RemoveAtSwap(Index);
			}
		}

		if (Instances.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

void UUIWidgetCache::HandleSceneChange()
{
	// A new scene loaded: free everything except session-persistent instances.
	for (auto It = ClosedInstances.CreateIterator(); It; ++It)
	{
		TArray<FUICachedInstance>& Instances = It.Value().Instances;
		Instances.RemoveAll(
			[](const FUICachedInstance& Cached)
			{
				return Cached.Policy != EUICachePolicy::KeepPersistent;
			});

		if (Instances.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}
