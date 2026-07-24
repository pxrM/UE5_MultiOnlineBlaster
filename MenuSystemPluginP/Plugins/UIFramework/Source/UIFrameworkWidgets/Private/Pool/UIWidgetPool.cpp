// Copyright TikiStar. All Rights Reserved.

#include "UIWidgetPool.h"
#include "UIPoolableWidget.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UUIWidgetPool::SetWorld(UWorld* InWorld)
{
	if (World.Get() != InWorld)
	{
		StopReaper();
		// Widgets are world-bound. Never hand an instance created in the old
		// world to callers operating in the new one.
		ReleaseAll();
		Reset();
	}
	World = InWorld;
	StartReaper();
}

float UUIWidgetPool::GetNow() const
{
	const UWorld* W = World.Get();
	return W ? W->GetTimeSeconds() : 0.f;
}

UUserWidget* UUIWidgetPool::CreateForClass(TSubclassOf<UUserWidget> WidgetClass)
{
	UWorld* W = World.Get();
	if (!W)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("WidgetPool: no world set. Call SetWorld first."));
		return nullptr;
	}
	return CreateWidget<UUserWidget>(W, WidgetClass);
}

void UUIWidgetPool::NotifyAcquired(UUserWidget* Widget)
{
	if (Widget && Widget->Implements<UUIPoolableWidget>())
	{
		IUIPoolableWidget::Execute_OnAcquiredFromPool(Widget);
	}
}

void UUIWidgetPool::NotifyReturned(UUserWidget* Widget)
{
	if (Widget && Widget->Implements<UUIPoolableWidget>())
	{
		IUIPoolableWidget::Execute_OnReturnedToPool(Widget);
	}
}

UUserWidget* UUIWidgetPool::Acquire(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("WidgetPool::Acquire: null WidgetClass."));
		return nullptr;
	}

	UUserWidget* Result = nullptr;

	// Reuse a hidden instance if one is free for this class.
	if (FUIWidgetPoolBucket* Bucket = FreeByClass.Find(WidgetClass))
	{
		while (Bucket->Entries.Num() > 0)
		{
			const FUIPooledWidget Entry = Bucket->Entries.Pop();
			if (Entry.Widget)
			{
				Entry.Widget->SetVisibility(ESlateVisibility::Visible);
				Result = Entry.Widget;
				break;
			}
		}
	}

	// None free: create a fresh one.
	if (!Result)
	{
		Result = CreateForClass(WidgetClass);
	}

	if (Result)
	{
		ActiveWidgets.AddUnique(Result);
		NotifyAcquired(Result);
	}
	return Result;
}

void UUIWidgetPool::Release(UUserWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	if (!ActiveWidgets.Contains(Widget))
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("WidgetPool::Release ignored widget '%s' because it is not currently acquired."),
			*GetNameSafe(Widget));
		return;
	}

	ActiveWidgets.RemoveSingleSwap(Widget);
	NotifyReturned(Widget);
	Widget->RemoveFromParent();

	FUIWidgetPoolBucket& Bucket = FreeByClass.FindOrAdd(Widget->GetClass());

	// Cap the idle set so a burst of releases can't grow memory without bound.
	if (MaxFreePerClass > 0 && Bucket.Entries.Num() >= MaxFreePerClass)
	{
		// Over capacity: drop it to GC instead of retaining.
		return;
	}

	Widget->SetVisibility(ESlateVisibility::Collapsed);
	Bucket.Entries.Add(FUIPooledWidget{ Widget, GetNow() });
}

void UUIWidgetPool::ReleaseAll()
{
	// Copy: Release mutates ActiveWidgets.
	TArray<TObjectPtr<UUserWidget>> Snapshot = ActiveWidgets;
	for (UUserWidget* Widget : Snapshot)
	{
		Release(Widget);
	}
	ActiveWidgets.Reset();
}

void UUIWidgetPool::Prewarm(TSubclassOf<UUserWidget> WidgetClass, int32 Count)
{
	if (!WidgetClass || Count <= 0)
	{
		return;
	}

	const float Now = GetNow();
	FUIWidgetPoolBucket& Bucket = FreeByClass.FindOrAdd(WidgetClass);
	for (int32 i = 0; i < Count; ++i)
	{
		if (MaxFreePerClass > 0 && Bucket.Entries.Num() >= MaxFreePerClass)
		{
			break;
		}
		if (UUserWidget* Widget = CreateForClass(WidgetClass))
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			Bucket.Entries.Add(FUIPooledWidget{ Widget, Now });
		}
	}
}

void UUIWidgetPool::TrimClass(TSubclassOf<UUserWidget> WidgetClass, int32 KeepCount)
{
	if (FUIWidgetPoolBucket* Bucket = FreeByClass.Find(WidgetClass))
	{
		const int32 Keep = FMath::Max(0, KeepCount);
		if (Bucket->Entries.Num() > Keep)
		{
			// Keep the most recently freed (tail) — they are the hottest.
			Bucket->Entries.RemoveAt(0, Bucket->Entries.Num() - Keep);
		}
	}
}

void UUIWidgetPool::ReapExpired()
{
	if (IdleReapSeconds <= 0.f)
	{
		return;
	}

	const float Now = GetNow();
	for (TPair<TSubclassOf<UUserWidget>, FUIWidgetPoolBucket>& Pair : FreeByClass)
	{
		TArray<FUIPooledWidget>& Entries = Pair.Value.Entries;
		for (int32 i = Entries.Num() - 1; i >= 0; --i)
		{
			if (!Entries[i].Widget || (Now - Entries[i].FreeTime) >= IdleReapSeconds)
			{
				Entries.RemoveAt(i);
			}
		}
	}
}

void UUIWidgetPool::Reset()
{
	FreeByClass.Empty();
}

int32 UUIWidgetPool::GetNumFree(TSubclassOf<UUserWidget> WidgetClass) const
{
	const FUIWidgetPoolBucket* Bucket = FreeByClass.Find(WidgetClass);
	return Bucket ? Bucket->Entries.Num() : 0;
}

void UUIWidgetPool::StartReaper()
{
	UWorld* W = World.Get();
	if (!W || IdleReapSeconds <= 0.f || ReapInterval <= 0.f)
	{
		return;
	}

	W->GetTimerManager().SetTimer(
		ReapTimerHandle, this, &UUIWidgetPool::ReapExpired, ReapInterval, /*bLoop=*/true);
}

void UUIWidgetPool::StopReaper()
{
	if (UWorld* W = World.Get())
	{
		W->GetTimerManager().ClearTimer(ReapTimerHandle);
	}
}

void UUIWidgetPool::BeginDestroy()
{
	StopReaper();
	Super::BeginDestroy();
}
