// Copyright TikiStar. All Rights Reserved.

#include "UIManagerSubsystem.h"
#include "UILayerSubsystem.h"
#include "UISettings.h"
#include "UIWidgetRegistry.h"
#include "UICoverageConfig.h"
#include "UIWidgetCache.h"
#include "UIManagedWidget.h"
#include "UIRootWidget.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectGlobals.h"

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
	bIsShuttingDown = false;
	bRejectNewOpens = false;
	Collection.InitializeDependency<UUILayerSubsystem>();

	// The manager owns scene ordering: close live widgets first, then evict
	// scene-scoped cache entries. This avoids delegate-registration coupling.
	MapChangedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UUIManagerSubsystem::HandleMapChanged);

	Cache = NewObject<UUIWidgetCache>(this);
	Cache->Initialize();

	if (UUILayerSubsystem* Layers = GetLayers())
	{
		Layers->OnWidgetRemoving().AddUObject(this, &UUIManagerSubsystem::HandleLayerWidgetRemoving);
		Layers->OnWidgetRemoved().AddUObject(this, &UUIManagerSubsystem::HandleLayerWidgetRemoved);
		Layers->OnWidgetActivationChanged().AddUObject(this, &UUIManagerSubsystem::HandleLayerWidgetActivationChanged);
	}
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

	UGameInstance* GameInstance = GetGameInstance();
	APlayerController* OwningPlayer = GameInstance
		? GameInstance->GetFirstLocalPlayerController()
		: nullptr;
	return Layers->InitializeRoot(RootClass, OwningPlayer);
}

const FUIWidgetEntry* UUIManagerSubsystem::ResolveEntry(FGameplayTag Key) const
{
	if (const FUIWidgetEntry* RuntimeEntry = RuntimeEntries.Find(Key))
	{
		return RuntimeEntry;
	}

	const UUISettings* Settings = GetDefault<UUISettings>();
	const UUIWidgetRegistry* Registry = (Settings && !Settings->Registry.IsNull())
		? Settings->Registry.LoadSynchronous()
		: nullptr;
	return Registry ? Registry->FindEntry(Key) : nullptr;
}

bool UUIManagerSubsystem::RegisterRuntimeEntry(
	FGameplayTag Key,
	const FUIWidgetEntry& Entry,
	bool bReplaceExisting,
	FString& OutError)
{
	OutError.Reset();
	TArray<FString> Errors;
	ValidateUIWidgetEntry(Key, Entry, Errors);
	if (!Errors.IsEmpty())
	{
		OutError = FString::Join(Errors, TEXT(" "));
		return false;
	}
	if (!bReplaceExisting && ResolveEntry(Key))
	{
		OutError = FString::Printf(TEXT("Registry key '%s' already exists."), *Key.ToString());
		return false;
	}

	CancelOpenUI(Key);
	if (Cache)
	{
		Cache->Remove(Key);
	}
	RuntimeEntries.Add(Key, Entry);
	return true;
}

bool UUIManagerSubsystem::UnregisterRuntimeEntry(FGameplayTag Key)
{
	if (!RuntimeEntries.Contains(Key))
	{
		return false;
	}
	CancelOpenUI(Key);
	if (Cache)
	{
		Cache->Remove(Key);
	}
	RuntimeEntries.Remove(Key);
	return true;
}

void UUIManagerSubsystem::ClearRuntimeEntries()
{
	TArray<FGameplayTag> Keys;
	RuntimeEntries.GetKeys(Keys);
	for (const FGameplayTag Key : Keys)
	{
		CancelOpenUI(Key);
		if (Cache)
		{
			Cache->Remove(Key);
		}
	}
	RuntimeEntries.Reset();
}

UUserWidget* UUIManagerSubsystem::OpenUI(FGameplayTag Key, UObject* Payload)
{
	if (bIsShuttingDown || bRejectNewOpens)
	{
		return nullptr;
	}
	// PushEntry applies the registry's single/multiple-instance policy.
	return PushEntry(Key, Payload);
}

void UUIManagerSubsystem::OpenUIAsync(FGameplayTag Key, FUIWidgetOpened OnOpened, UObject* Payload)
{
	if (bIsShuttingDown || bRejectNewOpens)
	{
		OnOpened.ExecuteIfBound(nullptr);
		return;
	}
	const FUIWidgetEntry* Entry = ResolveEntry(Key);
	if (Entry && !Entry->bAllowMultiple)
	{
		if (UUserWidget* Existing = GetLatestOpenWidget(Key))
		{
			OnOpened.ExecuteIfBound(Existing);
			return;
		}
	}

	const FSoftObjectPath ClassPath = Entry ? Entry->WidgetClass.ToSoftObjectPath() : FSoftObjectPath();
	if (!ClassPath.IsValid())
	{
		UE_LOG(LogUIFramework, Warning, TEXT("OpenUIAsync: key '%s' not registered or has no class."), *Key.ToString());
		OnOpened.ExecuteIfBound(nullptr);
		return;
	}

	FUIPendingOpenCall Call;
	Call.Payload = Payload;
	Call.Callback = OnOpened;
	PendingOpenCalls.FindOrAdd(Key).Calls.Add(MoveTemp(Call));
	if (PendingLoadHandles.Contains(Key))
	{
		return;
	}

	// Stream the class off the game thread; push once it is resident.
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TSharedPtr<FStreamableHandle> Handle = Streamable.RequestAsyncLoad(
		ClassPath,
		FStreamableDelegate::CreateUObject(this, &UUIManagerSubsystem::HandleAsyncLoaded, Key));

	if (Handle && PendingOpenCalls.Contains(Key))
	{
		PendingLoadHandles.Add(Key, MoveTemp(Handle));
	}
	else if (!Handle)
	{
		FUIPendingOpenCallList Calls;
		PendingOpenCalls.RemoveAndCopyValue(Key, Calls);
		for (FUIPendingOpenCall& PendingCall : Calls.Calls)
		{
			PendingCall.Callback.ExecuteIfBound(nullptr);
		}
	}
}

bool UUIManagerSubsystem::CancelOpenUI(FGameplayTag Key)
{
	TSharedPtr<FStreamableHandle> Handle;
	FUIPendingOpenCallList Calls;
	const bool bHadHandle = PendingLoadHandles.RemoveAndCopyValue(Key, Handle);
	const bool bHadCalls = PendingOpenCalls.RemoveAndCopyValue(Key, Calls);

	if (Handle)
	{
		Handle->CancelHandle();
	}
	for (FUIPendingOpenCall& PendingCall : Calls.Calls)
	{
		PendingCall.Callback.ExecuteIfBound(nullptr);
	}
	return bHadHandle || bHadCalls;
}

void UUIManagerSubsystem::HandleAsyncLoaded(FGameplayTag Key)
{
	PendingLoadHandles.Remove(Key);

	FUIPendingOpenCallList Calls;
	if (!PendingOpenCalls.RemoveAndCopyValue(Key, Calls))
	{
		return; // Request was cancelled after its completion was queued.
	}

	const FUIWidgetEntry* Entry = ResolveEntry(Key);

	if (Entry && Entry->bAllowMultiple)
	{
		for (FUIPendingOpenCall& PendingCall : Calls.Calls)
		{
			UUserWidget* Widget = PushEntry(Key, PendingCall.Payload);
			PendingCall.Callback.ExecuteIfBound(Widget);
		}
		return;
	}

	// Single-instance requests all observe the same resulting widget.
	UObject* Payload = Calls.Calls.IsEmpty() ? nullptr : Calls.Calls[0].Payload.Get();
	UUserWidget* Widget = PushEntry(Key, Payload);
	for (FUIPendingOpenCall& PendingCall : Calls.Calls)
	{
		PendingCall.Callback.ExecuteIfBound(Widget);
	}
}

UUserWidget* UUIManagerSubsystem::PushEntry(FGameplayTag Key, UObject* Payload)
{
	if (bIsShuttingDown || bRejectNewOpens)
	{
		return nullptr;
	}
	const FUIWidgetEntry* Entry = ResolveEntry(Key);
	if (!Entry)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: key '%s' not registered."), *Key.ToString());
		return nullptr;
	}
	if (Entry->bAllowMultiple && Entry->MaxOpenInstances > 0
		&& GetOpenCount(Key) >= Entry->MaxOpenInstances)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: key '%s' reached MaxOpenInstances (%d)."),
			*Key.ToString(), Entry->MaxOpenInstances);
		return nullptr;
	}

	if (!Entry->bAllowMultiple)
	{
		if (UUserWidget* Existing = GetLatestOpenWidget(Key))
		{
			return Existing;
		}
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
	bool bRestoredFromCache = false;

	// Reuse a kept-alive instance if the policy caches one (preserves its state).
	if (UUIWidgetCache::CachesInstance(Entry->CachePolicy))
	{
		if (UUserWidget* Reused = Cache->TakeInstance(Key, *Entry))
		{
			// Persistent instances may outlive the controller that originally owned
			// them. Rebind to the current root before reconstructing the widget.
			Reused->SetOwningPlayer(Layers->GetRoot()->GetOwningPlayer());
			Reused->SetVisibility(ESlateVisibility::Visible);
			Widget = Reused;
			bRestoredFromCache = true;
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

		Widget = CreateWidget<UUserWidget>(Layers->GetRoot(), WidgetClass);
	}

	if (!Widget)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushEntry: failed to create widget for key '%s'."), *Key.ToString());
		return nullptr;
	}

	if (!Layers->PushWidgetDeferred(Entry->Layer, Widget, Entry->bBlocksInput, Entry->bHandlesBack))
	{
		if (bRestoredFromCache)
		{
			Cache->ReturnInstance(Key, Widget, *Entry);
		}
		return nullptr;
	}

	OpenWidgets.FindOrAdd(Key).Widgets.Add(Widget);
	FUIOpenWidgetRecord& OpenRecord = OpenRecords.Add(Widget);
	OpenRecord.Key = Key;
	OpenRecord.Entry = *Entry;

	FUIOpenContext Context;
	Context.Key = Key;
	Context.Layer = Entry->Layer;
	Context.Payload = Payload;
	Context.bRestoredFromCache = bRestoredFromCache;

	if (Widget->Implements<UUIManagedWidget>())
	{
		if (bRestoredFromCache)
		{
			IUIManagedWidget::Execute_OnUIRestoredFromCache(Widget, Context);
		}
		IUIManagedWidget::Execute_OnUIOpening(Widget, Context);
	}
	WidgetOpeningEvent.Broadcast(Widget, Context);

	FGameplayTag TrackedKey;
	if (!FindTrackedKey(Widget, TrackedKey))
	{
		return nullptr; // A lifecycle callback closed the widget synchronously.
	}

	if (Widget->Implements<UUIManagedWidget>())
	{
		IUIManagedWidget::Execute_OnUIOpened(Widget, Context);
	}
	WidgetOpenedEvent.Broadcast(Widget, Context);

	if (!FindTrackedKey(Widget, TrackedKey))
	{
		return nullptr;
	}

	// Deferred refresh guarantees Opened precedes the first Activated callback.
	Layers->RefreshStack();
	return Widget;
}

bool UUIManagerSubsystem::CloseUI(FGameplayTag Key, UObject* Result)
{
	if (UUserWidget* Widget = GetLatestOpenWidget(Key))
	{
		return CloseTrackedWidget(Key, Widget, Result);
	}
	return CancelOpenUI(Key);
}

bool UUIManagerSubsystem::CloseWidget(UUserWidget* Widget, UObject* Result)
{
	if (!Widget)
	{
		return false;
	}

	FGameplayTag TrackedKey;
	return FindTrackedKey(Widget, TrackedKey) && CloseTrackedWidget(TrackedKey, Widget, Result);
}

UUserWidget* UUIManagerSubsystem::GetLatestOpenWidget(FGameplayTag Key) const
{
	if (const FUIOpenWidgetList* List = OpenWidgets.Find(Key))
	{
		for (int32 Index = List->Widgets.Num() - 1; Index >= 0; --Index)
		{
			if (IsValid(List->Widgets[Index]))
			{
				return List->Widgets[Index];
			}
		}
	}
	return nullptr;
}

bool UUIManagerSubsystem::FindTrackedKey(const UUserWidget* Widget, FGameplayTag& OutKey) const
{
	if (!Widget)
	{
		return false;
	}

	if (const FUIOpenWidgetRecord* Record = OpenRecords.Find(const_cast<UUserWidget*>(Widget)))
	{
		OutKey = Record->Key;
		return true;
	}
	return false;
}

bool UUIManagerSubsystem::CloseTrackedWidget(
	FGameplayTag Key,
	UUserWidget* Widget,
	UObject* Result,
	EUIWidgetCloseReason Reason)
{
	UUILayerSubsystem* Layers = GetLayers();
	FUIOpenWidgetRecord* Record = OpenRecords.Find(Widget);
	FUIOpenWidgetList* List = OpenWidgets.Find(Key);
	if (!Layers || !Record || Record->Key != Key || !List || !Widget || !List->Widgets.Contains(Widget))
	{
		return false;
	}
	if (ClosingContexts.Contains(Widget))
	{
		return false; // Ignore re-entrant close attempts from lifecycle callbacks.
	}

	FUICloseContext Context;
	Context.Key = Key;
	Context.Layer = Record->Entry.Layer;
	Context.Reason = Reason;
	Context.Result = Result;

	Context.bWillBeCached = UUIWidgetCache::CachesInstance(Record->Entry.CachePolicy);
	ClosingContexts.Add(Widget, Context);

	// Removing/Removed delegates synchronously own lifecycle calls, untracking and caching.
	if (!Layers->RemoveWidgetWithReason(Record->Entry.Layer, Widget, Reason))
	{
		ClosingContexts.Remove(Widget);
		return false;
	}
	return true;
}

void UUIManagerSubsystem::HandleLayerWidgetRemoving(
	UUserWidget* Widget,
	EUILayer Layer,
	EUIWidgetCloseReason Reason)
{
	if (!Widget)
	{
		return;
	}

	FGameplayTag TrackedKey;
	if (!FindTrackedKey(Widget, TrackedKey))
	{
		return;
	}

	FUICloseContext* ExistingContext = ClosingContexts.Find(Widget);
	if (!ExistingContext)
	{
		const FUIOpenWidgetRecord* Record = OpenRecords.Find(Widget);
		FUICloseContext Context;
		Context.Key = TrackedKey;
		Context.Layer = Record ? Record->Entry.Layer : Layer;
		Context.Reason = Reason;
		Context.bWillBeCached = Record && UUIWidgetCache::CachesInstance(Record->Entry.CachePolicy);
		ExistingContext = &ClosingContexts.Add(Widget, Context);
	}

	if (Widget->Implements<UUIManagedWidget>())
	{
		IUIManagedWidget::Execute_OnUIClosing(Widget, *ExistingContext);
	}
	WidgetClosingEvent.Broadcast(Widget, *ExistingContext);
}

void UUIManagerSubsystem::HandleLayerWidgetActivationChanged(
	UUserWidget* Widget,
	EUILayer Layer,
	bool bIsActive)
{
	FGameplayTag TrackedKey;
	if (!Widget || !FindTrackedKey(Widget, TrackedKey))
	{
		return;
	}

	if (Widget->Implements<UUIManagedWidget>())
	{
		if (bIsActive)
		{
			IUIManagedWidget::Execute_OnUIActivated(Widget);
		}
		else
		{
			IUIManagedWidget::Execute_OnUIDeactivated(Widget);
		}
	}
	WidgetActivationChangedEvent.Broadcast(Widget, TrackedKey, bIsActive);
}

void UUIManagerSubsystem::HandleLayerWidgetRemoved(UUserWidget* Widget, EUILayer Layer)
{
	if (!Widget)
	{
		return;
	}

	FGameplayTag TrackedKey;
	if (!FindTrackedKey(Widget, TrackedKey))
	{
		ClosingContexts.Remove(Widget);
		return; // Widget was pushed directly through the low-level layer API.
	}

	FUIOpenWidgetList* List = OpenWidgets.Find(TrackedKey);
	FUIOpenWidgetRecord Record;
	const bool bHadRecord = OpenRecords.RemoveAndCopyValue(Widget, Record);
	if (!List)
	{
		ClosingContexts.Remove(Widget);
		return;
	}

	FUICloseContext Context;
	if (!ClosingContexts.RemoveAndCopyValue(Widget, Context))
	{
		Context.Key = TrackedKey;
		Context.Layer = Layer;
	}

	// The widget is closed from the manager's perspective before user lifecycle
	// code runs. This makes OpenUI(Key) from OnUIClosed create/restore a new cycle
	// instead of returning the instance that has just left the stack.
	List->Widgets.RemoveSingle(Widget);
	if (List->Widgets.IsEmpty())
	{
		OpenWidgets.Remove(TrackedKey);
	}

	if (Widget->Implements<UUIManagedWidget>())
	{
		IUIManagedWidget::Execute_OnUIClosed(Widget, Context);
	}
	WidgetClosedEvent.Broadcast(Widget, Context);

	if (Cache)
	{
		if (bHadRecord)
		{
			if (Record.Entry.Layer != Layer)
			{
				UE_LOG(LogUIFramework, Warning,
					TEXT("Widget '%s' was removed from layer %d but registry key '%s' expects layer %d."),
					*GetNameSafe(Widget), static_cast<int32>(Layer), *TrackedKey.ToString(), static_cast<int32>(Record.Entry.Layer));
			}
			Cache->ReturnInstance(TrackedKey, Widget, Record.Entry);
		}
	}
}

void UUIManagerSubsystem::CloseAllUI()
{
	CloseAllUIWithReason(EUIWidgetCloseReason::Requested);
}

void UUIManagerSubsystem::ForceCloseAllUI()
{
	TGuardValue<bool> RejectOpenGuard(bRejectNewOpens, true);
	CloseAllUIWithReason(EUIWidgetCloseReason::Requested);
}

void UUIManagerSubsystem::CloseAllUIWithReason(EUIWidgetCloseReason Reason)
{
	struct FPendingClose
	{
		FGameplayTag Key;
		TObjectPtr<UUserWidget> Widget;
	};

	TArray<FGameplayTag> PendingKeys;
	PendingOpenCalls.GetKeys(PendingKeys);
	for (const FGameplayTag& PendingKey : PendingKeys)
	{
		CancelOpenUI(PendingKey);
	}

	TArray<FPendingClose> Pending;
	for (const TPair<FGameplayTag, FUIOpenWidgetList>& Pair : OpenWidgets)
	{
		for (UUserWidget* Widget : Pair.Value.Widgets)
		{
			Pending.Add({ Pair.Key, Widget });
		}
	}

	for (int32 Index = Pending.Num() - 1; Index >= 0; --Index)
	{
		CloseTrackedWidget(Pending[Index].Key, Pending[Index].Widget, nullptr, Reason);
	}
}

void UUIManagerSubsystem::HandleMapChanged(UWorld* LoadedWorld)
{
	if (bIsShuttingDown || !LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	TGuardValue<bool> RejectOpenGuard(bRejectNewOpens, true);

	// A root created with an owning player cannot safely retain that controller
	// across non-seamless travel. Close first so lifecycle and cache policy run,
	// then lazily create a root for the new local player on the next open.
	CloseAllUIWithReason(EUIWidgetCloseReason::SceneChange);
	if (Cache)
	{
		Cache->HandleSceneChange();
	}
	if (UUILayerSubsystem* Layers = GetLayers())
	{
		Layers->TeardownRoot();
	}
}

bool UUIManagerSubsystem::HandleBackAction()
{
	if (UUILayerSubsystem* Layers = GetLayers())
	{
		return Layers->HandleBackAction();
	}
	return false;
}

bool UUIManagerSubsystem::IsUIOpen(FGameplayTag Key) const
{
	return GetOpenCount(Key) > 0;
}

int32 UUIManagerSubsystem::GetOpenCount(FGameplayTag Key) const
{
	const FUIOpenWidgetList* List = OpenWidgets.Find(Key);
	return List ? List->Widgets.Num() : 0;
}

bool UUIManagerSubsystem::ValidateConfiguration(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();
	const UUISettings* Settings = GetDefault<UUISettings>();
	if (!Settings)
	{
		OutErrors.Add(TEXT("UI Framework settings are unavailable."));
		return false;
	}

	if (Settings->Registry.IsNull())
	{
		if (RuntimeEntries.IsEmpty())
		{
			OutErrors.Add(TEXT("UI Framework Registry is not configured and no runtime entries are registered."));
		}
	}
	else if (const UUIWidgetRegistry* Registry = Settings->Registry.LoadSynchronous())
	{
		Registry->ValidateEntries(OutErrors);
	}
	else
	{
		OutErrors.Add(TEXT("UI Framework Registry failed to load."));
	}
	for (const TPair<FGameplayTag, FUIWidgetEntry>& Pair : RuntimeEntries)
	{
		ValidateUIWidgetEntry(Pair.Key, Pair.Value, OutErrors);
	}

	if (!Settings->CoverageConfig.IsNull())
	{
		if (const UUICoverageConfig* Coverage = Settings->CoverageConfig.LoadSynchronous())
		{
			Coverage->ValidateRules(OutErrors);
		}
		else
		{
			OutErrors.Add(TEXT("UI Framework CoverageConfig failed to load."));
		}
	}

	return OutErrors.IsEmpty();
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
	bIsShuttingDown = true;
	if (MapChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapChangedHandle);
		MapChangedHandle.Reset();
	}
	CloseAllUIWithReason(EUIWidgetCloseReason::SubsystemShutdown);

	if (UUILayerSubsystem* Layers = GetLayers())
	{
		Layers->OnWidgetRemoving().RemoveAll(this);
		Layers->OnWidgetRemoved().RemoveAll(this);
		Layers->OnWidgetActivationChanged().RemoveAll(this);
	}

	TArray<FGameplayTag> PendingKeys;
	PendingLoadHandles.GetKeys(PendingKeys);
	for (const FGameplayTag& Key : PendingKeys)
	{
		if (TSharedPtr<FStreamableHandle>* Handle = PendingLoadHandles.Find(Key))
		{
			if (*Handle)
			{
				(*Handle)->CancelHandle();
			}
		}
	}
	PendingLoadHandles.Reset();
	PendingOpenCalls.Reset();
	RuntimeEntries.Reset();

	if (Cache)
	{
		Cache->Shutdown();
		Cache = nullptr;
	}
	OpenWidgets.Reset();
	OpenRecords.Reset();
	ClosingContexts.Reset();
	Super::Deinitialize();
}
