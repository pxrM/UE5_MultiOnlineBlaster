// Copyright TikiStar. All Rights Reserved.

#include "UIEventBusSubsystem.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UUIEventBusSubsystem* UUIEventBusSubsystem::Get(const UObject* WorldContextObject)
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
		return GameInstance->GetSubsystem<UUIEventBusSubsystem>();
	}
	return nullptr;
}

void UUIEventBusSubsystem::Subscribe(FGameplayTag Channel, const FUIEventHandler& Handler)
{
	if (!Channel.IsValid() || !Handler.IsBound())
	{
		UE_LOG(LogUIFramework, Warning, TEXT("Subscribe: invalid channel or unbound handler."));
		return;
	}
	Channels.FindOrAdd(Channel).AddUnique(Handler);
}

void UUIEventBusSubsystem::Unsubscribe(FGameplayTag Channel, const FUIEventHandler& Handler)
{
	if (FUIEventMulticast* Multicast = Channels.Find(Channel))
	{
		Multicast->Remove(Handler);
		if (!Multicast->IsBound())
		{
			Channels.Remove(Channel);
		}
	}
}

void UUIEventBusSubsystem::UnsubscribeAll(UObject* Subscriber)
{
	if (!Subscriber)
	{
		return;
	}
	for (auto It = Channels.CreateIterator(); It; ++It)
	{
		It.Value().RemoveAll(Subscriber);
		if (!It.Value().IsBound())
		{
			It.RemoveCurrent();
		}
	}
}

void UUIEventBusSubsystem::Broadcast(FGameplayTag Channel, UObject* Payload)
{
	if (FUIEventMulticast* Multicast = Channels.Find(Channel))
	{
		// Subscribers may add/remove channels from inside their callback. Broadcast
		// a snapshot so TMap mutation cannot invalidate the delegate being invoked.
		FUIEventMulticast Snapshot = *Multicast;
		Snapshot.Broadcast(Payload);
	}
}

void UUIEventBusSubsystem::Deinitialize()
{
	Channels.Empty();
	Super::Deinitialize();
}
