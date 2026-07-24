// Copyright TikiStar. All Rights Reserved.

#include "UIScriptBridgeSubsystem.h"
#include "UIScriptAsyncRequest.h"
#include "UIManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"

UUIScriptBridgeSubsystem* UUIScriptBridgeSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UUIScriptBridgeSubsystem>() : nullptr;
}

void UUIScriptBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UUIManagerSubsystem>();
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		Manager->OnWidgetOpening().AddUObject(this, &UUIScriptBridgeSubsystem::HandleWidgetOpening);
		Manager->OnWidgetOpened().AddUObject(this, &UUIScriptBridgeSubsystem::HandleWidgetOpened);
		Manager->OnWidgetClosing().AddUObject(this, &UUIScriptBridgeSubsystem::HandleWidgetClosing);
		Manager->OnWidgetClosed().AddUObject(this, &UUIScriptBridgeSubsystem::HandleWidgetClosed);
		Manager->OnWidgetActivationChanged().AddUObject(this, &UUIScriptBridgeSubsystem::HandleWidgetActivationChanged);
	}
}

void UUIScriptBridgeSubsystem::Deinitialize()
{
	TArray<TObjectPtr<UUIScriptAsyncRequest>> Requests;
	ActiveRequests.GenerateValueArray(Requests);
	ActiveRequests.Reset();
	for (UUIScriptAsyncRequest* Request : Requests)
	{
		if (Request)
		{
			Request->Finish(nullptr, EUIScriptRequestState::Cancelled);
		}
	}

	if (UUIManagerSubsystem* Manager = GetManager())
	{
		Manager->OnWidgetOpening().RemoveAll(this);
		Manager->OnWidgetOpened().RemoveAll(this);
		Manager->OnWidgetClosing().RemoveAll(this);
		Manager->OnWidgetClosed().RemoveAll(this);
		Manager->OnWidgetActivationChanged().RemoveAll(this);
	}
	Super::Deinitialize();
}

UUIManagerSubsystem* UUIScriptBridgeSubsystem::GetManager() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UUIManagerSubsystem>() : nullptr;
}

UUserWidget* UUIScriptBridgeSubsystem::OpenUI(FGameplayTag Key, UObject* Payload)
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		return Manager->OpenUI(Key, Payload);
	}
	return nullptr;
}

UUIScriptAsyncRequest* UUIScriptBridgeSubsystem::OpenUIAsync(FGameplayTag Key, UObject* Payload)
{
	UUIScriptAsyncRequest* Request = NewObject<UUIScriptAsyncRequest>(this);
	const int32 RequestId = NextRequestId++;
	Request->InitializeRequest(this, RequestId, Key);
	ActiveRequests.Add(RequestId, Request);

	if (UUIManagerSubsystem* Manager = GetManager())
	{
		FUIWidgetOpened Callback;
		Callback.BindDynamic(Request, &UUIScriptAsyncRequest::HandleOpened);
		Manager->OpenUIAsync(Key, Callback, Payload);
	}
	else
	{
		CompleteRequest(Request, nullptr);
	}
	return Request;
}

void UUIScriptBridgeSubsystem::CancelRequest(UUIScriptAsyncRequest* Request)
{
	if (!Request || !ActiveRequests.Remove(Request->GetRequestId()))
	{
		return;
	}
	// Individual script cancellation suppresses this listener. The manager's shared
	// class load remains alive for other callers waiting on the same key.
	Request->Finish(nullptr, EUIScriptRequestState::Cancelled);
}

void UUIScriptBridgeSubsystem::CompleteRequest(UUIScriptAsyncRequest* Request, UUserWidget* Widget)
{
	if (!Request || !ActiveRequests.Remove(Request->GetRequestId()))
	{
		return;
	}
	Request->Finish(Widget, Widget ? EUIScriptRequestState::Succeeded : EUIScriptRequestState::Failed);
}

bool UUIScriptBridgeSubsystem::CloseUI(FGameplayTag Key, UObject* Result)
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		return Manager->CloseUI(Key, Result);
	}
	return false;
}

bool UUIScriptBridgeSubsystem::CloseWidget(UUserWidget* Widget, UObject* Result)
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		return Manager->CloseWidget(Widget, Result);
	}
	return false;
}

void UUIScriptBridgeSubsystem::ForceCloseAllUI()
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		Manager->ForceCloseAllUI();
	}
}

bool UUIScriptBridgeSubsystem::HandleBackAction()
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		return Manager->HandleBackAction();
	}
	return false;
}

bool UUIScriptBridgeSubsystem::ValidateConfiguration(TArray<FString>& OutErrors) const
{
	if (const UUIManagerSubsystem* Manager = GetManager())
	{
		return Manager->ValidateConfiguration(OutErrors);
	}
	OutErrors.Reset();
	OutErrors.Add(TEXT("UI Manager subsystem is unavailable."));
	return false;
}

bool UUIScriptBridgeSubsystem::RegisterRuntimeWidget(
	const FString& KeyName,
	const FString& WidgetClassPath,
	EUILayer Layer,
	EUICachePolicy CachePolicy,
	bool bAllowMultiple,
	bool bBlocksInput,
	bool bHandlesBack,
	int32 MaxOpenInstances,
	int32 MaxCachedInstances,
	float IdleTimeoutSeconds,
	bool bReplaceExisting,
	FString& OutError)
{
	OutError.Reset();
	const FGameplayTag Key = UGameplayTagsManager::Get().RequestGameplayTag(FName(*KeyName), false);
	if (!Key.IsValid())
	{
		OutError = FString::Printf(
			TEXT("GameplayTag '%s' is not registered. Add it to DefaultGameplayTags.ini first."),
			*KeyName);
		return false;
	}

	const FSoftObjectPath ClassPath(WidgetClassPath);
	if (!ClassPath.IsValid())
	{
		OutError = FString::Printf(TEXT("WidgetClass '%s' is not a valid object path."), *WidgetClassPath);
		return false;
	}

	UUIManagerSubsystem* Manager = GetManager();
	if (!Manager)
	{
		OutError = TEXT("UI Manager subsystem is unavailable.");
		return false;
	}

	FUIWidgetEntry Entry;
	Entry.WidgetClass = TSoftClassPtr<UUserWidget>(ClassPath);
	Entry.Layer = Layer;
	Entry.CachePolicy = CachePolicy;
	Entry.bAllowMultiple = bAllowMultiple;
	Entry.bBlocksInput = bBlocksInput;
	Entry.bHandlesBack = bHandlesBack;
	Entry.MaxOpenInstances = MaxOpenInstances;
	Entry.MaxCachedInstances = MaxCachedInstances;
	Entry.IdleTimeoutSeconds = IdleTimeoutSeconds;
	return Manager->RegisterRuntimeEntry(Key, Entry, bReplaceExisting, OutError);
}

bool UUIScriptBridgeSubsystem::UnregisterRuntimeWidget(const FString& KeyName, FString& OutError)
{
	OutError.Reset();
	const FGameplayTag Key = UGameplayTagsManager::Get().RequestGameplayTag(FName(*KeyName), false);
	if (!Key.IsValid())
	{
		OutError = FString::Printf(TEXT("GameplayTag '%s' is not registered."), *KeyName);
		return false;
	}
	UUIManagerSubsystem* Manager = GetManager();
	if (!Manager)
	{
		OutError = TEXT("UI Manager subsystem is unavailable.");
		return false;
	}
	if (!Manager->UnregisterRuntimeEntry(Key))
	{
		OutError = FString::Printf(TEXT("Runtime registry key '%s' does not exist."), *KeyName);
		return false;
	}
	return true;
}

void UUIScriptBridgeSubsystem::ClearRuntimeWidgets()
{
	if (UUIManagerSubsystem* Manager = GetManager())
	{
		Manager->ClearRuntimeEntries();
	}
}

int32 UUIScriptBridgeSubsystem::GetRuntimeWidgetCount() const
{
	const UUIManagerSubsystem* Manager = GetManager();
	return Manager ? Manager->GetRuntimeEntryCount() : 0;
}

void UUIScriptBridgeSubsystem::HandleWidgetOpening(UUserWidget* Widget, const FUIOpenContext& Context)
{
	OnWidgetOpening.Broadcast(Widget, Context);
}

void UUIScriptBridgeSubsystem::HandleWidgetOpened(UUserWidget* Widget, const FUIOpenContext& Context)
{
	OnWidgetOpened.Broadcast(Widget, Context);
}

void UUIScriptBridgeSubsystem::HandleWidgetClosing(UUserWidget* Widget, const FUICloseContext& Context)
{
	OnWidgetClosing.Broadcast(Widget, Context);
}

void UUIScriptBridgeSubsystem::HandleWidgetClosed(UUserWidget* Widget, const FUICloseContext& Context)
{
	OnWidgetClosed.Broadcast(Widget, Context);
}

void UUIScriptBridgeSubsystem::HandleWidgetActivationChanged(UUserWidget* Widget, FGameplayTag Key, bool bIsActive)
{
	OnWidgetActivationChanged.Broadcast(Widget, Key, bIsActive);
}
