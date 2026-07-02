// Copyright TikiStar. All Rights Reserved.

#include "UIManagerSubsystem.h"
#include "UILayerSubsystem.h"
#include "UISettings.h"
#include "UIWidgetRegistry.h"
#include "UIRootWidget.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"
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

	const UUISettings* Settings = GetDefault<UUISettings>();
	if (!Settings || Settings->Registry.IsNull())
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUI: Registry unset in UI Framework settings."));
		return nullptr;
	}

	const UUIWidgetRegistry* Registry = Settings->Registry.LoadSynchronous();
	if (!Registry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUI: failed to load Registry."));
		return nullptr;
	}

	const FUIWidgetEntry* Entry = Registry->FindEntry(Key);
	if (!Entry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUI: key '%s' not registered."), *Key.ToString());
		return nullptr;
	}

	TSubclassOf<UUserWidget> WidgetClass = Entry->WidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUI: key '%s' has no loadable WidgetClass."), *Key.ToString());
		return nullptr;
	}

	if (!EnsureRoot())
	{
		return nullptr;
	}

	UUILayerSubsystem* Layers = GetLayers();
	UUserWidget* Widget = Layers ? Layers->PushToLayer(Entry->Layer, WidgetClass) : nullptr;
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

	const bool bRemoved = Layers->RemoveWidget(*Layer, *Widget);
	OpenWidgets.Remove(Key);
	OpenLayers.Remove(Key);
	return bRemoved;
}
