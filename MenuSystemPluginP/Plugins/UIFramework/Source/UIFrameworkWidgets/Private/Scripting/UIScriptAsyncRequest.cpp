// Copyright TikiStar. All Rights Reserved.

#include "UIScriptAsyncRequest.h"
#include "UIScriptBridgeSubsystem.h"
#include "Async/Async.h"
#include "Blueprint/UserWidget.h"

void UUIScriptAsyncRequest::InitializeRequest(
	UUIScriptBridgeSubsystem* InOwner,
	int32 InRequestId,
	FGameplayTag InKey)
{
	Owner = InOwner;
	RequestId = InRequestId;
	Key = InKey;
	State = EUIScriptRequestState::Pending;
}

void UUIScriptAsyncRequest::Cancel()
{
	if (State == EUIScriptRequestState::Pending && Owner)
	{
		Owner->CancelRequest(this);
	}
}

void UUIScriptAsyncRequest::HandleOpened(UUserWidget* InWidget)
{
	// Manager may complete synchronously for an already-open UI. Defer delivery so
	// scripts have a chance to bind OnCompleted after OpenUIAsync returns.
	TWeakObjectPtr<UUIScriptAsyncRequest> WeakThis(this);
	TWeakObjectPtr<UUserWidget> WeakWidget(InWidget);
	AsyncTask(ENamedThreads::GameThread, [WeakThis, WeakWidget]()
	{
		if (UUIScriptAsyncRequest* Request = WeakThis.Get())
		{
			if (UUIScriptBridgeSubsystem* Bridge = Request->Owner.Get())
			{
				Bridge->CompleteRequest(Request, WeakWidget.Get());
			}
		}
	});
}

void UUIScriptAsyncRequest::Finish(UUserWidget* InWidget, EUIScriptRequestState InState)
{
	if (State != EUIScriptRequestState::Pending)
	{
		return;
	}
	Widget = InWidget;
	State = InState;
	Owner = nullptr;
	OnCompleted.Broadcast(this, Widget, State == EUIScriptRequestState::Succeeded);
}
