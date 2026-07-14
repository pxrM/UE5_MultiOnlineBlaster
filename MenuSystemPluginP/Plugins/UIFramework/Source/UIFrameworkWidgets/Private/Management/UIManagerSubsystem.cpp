// Copyright TikiStar. All Rights Reserved.

#include "UIManagerSubsystem.h"
#include "UILayerSubsystem.h"
#include "UISettings.h"
#include "UIWidgetRegistry.h"
#include "UIWidgetCache.h"
#include "UIRootWidget.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Blueprint/UserWidget.h"

UUIManagerSubsystem* UUIManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		return GameInstance->GetSubsystem<UUIManagerSubsystem>();
	}
	return nullptr;
}

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Cache = NewObject<UUIWidgetCache>(this);
	Cache->Initialize();
}

UUILayerSubsystem* UUIManagerSubsystem::GetLayers() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UUILayerSubsystem>() : nullptr;
}

UUIRootWidget* UUIManagerSubsystem::EnsureRoot()
{
	UUILayerSubsystem* Layers = GetLayers();
	if (!Layers)
	{
		return nullptr;
	}

	if (UUIRootWidget* Existing = Layers->GetRoot())
	{
		return Existing;
	}

	const UUISettings* Settings = GetDefault<UUISettings>();
	if (!Settings || Settings->DefaultRootClass.IsNull())
	{
		UE_LOG(LogUIFramework, Warning, TEXT("EnsureRoot: DefaultRootClass unset in UI Framework settings."));
		return nullptr;
	}

	// Synchronous load: the root is needed immediately. It is loaded once and cached.
	TSubclassOf<UUIRootWidget> RootClass = Settings->DefaultRootClass.LoadSynchronous();
	if (!RootClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("EnsureRoot: failed to load DefaultRootClass."));
		return nullptr;
	}

	return Layers->InitializeRoot(RootClass);
}

UUserWidget* UUIManagerSubsystem::OpenUI(FGameplayTag Key)
{
	// Already open and single-instance: return the existing widget.
	if (TObjectPtr<UUserWidget>* Existing = OpenWidgets.Find(Key))
	{
		return *Existing;
	}
	// Synchronous: PushEntry loads the class inline.
	return PushEntry(Key);
}

void UUIManagerSubsystem::OpenUIAsync(FGameplayTag Key, FUIWidgetOpened OnOpened)
{
	// Already open and single-instance: hand back the existing instance now.
	if (TObjectPtr<UUserWidget>* Existing = OpenWidgets.Find(Key))
	{
		OnOpened.ExecuteIfBound(*Existing);
		return;
	}

	const UUISettings* Settings = GetDefault<UUISettings>();
	const UUIWidgetRegistry* Registry = (Settings && !Settings->Registry.IsNull()) ? Settings->Registry.LoadSynchronous() : nullptr;
	if (!Registry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUIAsync: Registry unset or failed to load."));
		OnOpened.ExecuteIfBound(nullptr);
		return;
	}

	const FUIWidgetEntry* Entry = Registry->FindEntry(Key);
	const FSoftObjectPath ClassPath = Entry ? Entry->WidgetClass.ToSoftObjectPath() : FSoftObjectPath();
	if (!ClassPath.IsValid())
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUIAsync: key '%s' not registered or has no class."), *Key.ToString());
		OnOpened.ExecuteIfBound(nullptr);
		return;
	}

	// Stream the class off the game thread; push once it is resident.
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(
		ClassPath,
		FStreamableDelegate::CreateUObject(this, &UUIManagerSubsystem::HandleAsyncLoaded, Key, OnOpened));
}

void UUIManagerSubsystem::HandleAsyncLoaded(FGameplayTag Key, FUIWidgetOpened OnOpened)
{
	// Class is resident now; PushEntry's LoadSynchronous resolves instantly.
	UUserWidget* Widget = PushEntry(Key);
	OnOpened.ExecuteIfBound(Widget);
}

UUserWidget* UUIManagerSubsystem::PushEntry(FGameplayTag Key)
{
	const UUISettings* Settings = GetDefault<UUISettings>();
	const UUIWidgetRegistry* Registry = (Settings && !Settings->Registry.IsNull()) ? Settings->Registry.LoadSynchronous() : nullptr;
	if (!Registry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: Registry unset or failed to load."));
		return nullptr;
	}

	const FUIWidgetEntry* Entry = Registry->FindEntry(Key);
	if (!Entry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: key '%s' not registered."), *Key.ToString());
		return nullptr;
	}

	if (!EnsureRoot())
	{
		return nullptr;
	}

	UUILayerSubsystem* Layers = GetLayers();
	if (!Layers || !Cache)
	{
		return nullptr;
	}

	UUserWidget* Widget = nullptr;

	// Reuse a kept-alive instance if the policy caches one (preserves its state).
	if (UUIWidgetCache::CachesInstance(Entry->CachePolicy))
	{
		if (UUserWidget* Reused = Cache->TakeInstance(Key))
		{
			Reused->SetVisibility(ESlateVisibility::Visible);
			Widget = Layers->PushWidget(Entry->Layer, Reused);
		}
	}

	// Otherwise (or first open) create via the resolved (cached) class.
	if (!Widget)
	{
		TSubclassOf<UUserWidget> WidgetClass = Cache->ResolveClass(Key, *Entry);
		if (!WidgetClass)
		{
			UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: key '%s' has no loadable WidgetClass."), *Key.ToString());
			return nullptr;
		}
		Widget = Layers->PushToLayer(Entry->Layer, WidgetClass);
	}

	if (Widget && !Entry->bAllowMultiple)
	{
		// Track single-instance UI so CloseUI can find it and OpenUI can dedupe.
		OpenWidgets.Add(Key, Widget);
		OpenLayers.Add(Key, Entry->Layer);
	}
	return Widget;
}

bool UUIManagerSubsystem::CloseUI(FGameplayTag Key)
{
	UUILayerSubsystem* Layers = GetLayers();
	if (!Layers)
	{
		return false;
	}

	TObjectPtr<UUserWidget>* Widget = OpenWidgets.Find(Key);
	EUILayer* Layer = OpenLayers.Find(Key);
	if (!Widget || !Layer)
	{
		return false;
	}

	// Remove from the layer stack + viewport, then let the cache decide the instance's
	// fate by policy: keep-alive (hidden, reusable) or drop to GC.
	UUserWidget* ClosedWidget = *Widget;
	const bool bRemoved = Layers->RemoveWidget(*Layer, ClosedWidget);

	OpenWidgets.Remove(Key);
	OpenLayers.Remove(Key);

	if (Cache)
	{
		const UUISettings* Settings = GetDefault<UUISettings>();
		const UUIWidgetRegistry* Registry = (Settings && !Settings->Registry.IsNull()) ? Settings->Registry.LoadSynchronous() : nullptr;
		if (const FUIWidgetEntry* Entry = Registry ? Registry->FindEntry(Key) : nullptr)
		{
			Cache->ReturnInstance(Key, ClosedWidget, *Entry);
		}
	}

	return bRemoved;
}

void UUIManagerSubsystem::ClearCache()
{
	if (Cache)
	{
		Cache->ClearAll();
	}
}

void UUIManagerSubsystem::Deinitialize()
{
	if (Cache)
	{
		Cache->Shutdown();
		Cache = nullptr;
	}
	OpenWidgets.Reset();
	OpenLayers.Reset();
	Super::Deinitialize();
}
