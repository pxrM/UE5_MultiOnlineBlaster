// Copyright TikiStar. All Rights Reserved.

#include "UIEventBusSubsystem.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"

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
	Channels.FindOrAdd(Channel).Add(Handler);
}

void UUIEventBusSubsystem::Unsubscribe(FGameplayTag Channel, const FUIEventHandler& Handler)
{
	if (FUIEventMulticast* Multicast = Channels.Find(Channel))
	{
		Multicast->Remove(Handler);
	}
}

void UUIEventBusSubsystem::UnsubscribeAll(UObject* Subscriber)
{
	if (!Subscriber)
	{
		return;
	}
	for (TPair<FGameplayTag, FUIEventMulticast>& Pair : Channels)
	{
		Pair.Value.RemoveAll(Subscriber);
	}
}

void UUIEventBusSubsystem::Broadcast(FGameplayTag Channel, UObject* Payload)
{
	if (FUIEventMulticast* Multicast = Channels.Find(Channel))
	{
		Multicast->Broadcast(Payload);
	}
}

void UUIEventBusSubsystem::Deinitialize()
{
	Channels.Empty();
	Super::Deinitialize();
}
