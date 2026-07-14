// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "UIEventBusSubsystem.generated.h"

/** Handler bound to a channel. Payload is an optional data object (may be null). */
DECLARE_DYNAMIC_DELEGATE_OneParam(FUIEventHandler, UObject*, Payload);

/** Internal per-channel multicast of subscribed handlers. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIEventMulticast, UObject*, Payload);

/**
 * Tag-keyed publish/subscribe bus for UI. Lives in Core (no UMG) so any layer —
 * widgets, view models, gameplay — can talk without referencing each other.
 * Broadcast on a channel; every subscriber to that GameplayTag gets the payload.
 * See Docs/DESIGN.md section 2 (decoupling).
 *
 *   UUIEventBusSubsystem::Get(this)->Broadcast(TAG_UI_CoinsChanged, CoinData);
 */
UCLASS()
class UIFRAMEWORKCORE_API UUIEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Convenience accessor from any object with a world. Returns nullptr if unavailable. */
	UFUNCTION(BlueprintCallable, Category = "UI|Events", meta = (WorldContext = "WorldContextObject"))
	static UUIEventBusSubsystem* Get(const UObject* WorldContextObject);

	/** Subscribe a handler to a channel. Duplicate handlers are ignored. */
	UFUNCTION(BlueprintCallable, Category = "UI|Events")
	void Subscribe(FGameplayTag Channel, const FUIEventHandler& Handler);

	/** Remove a previously subscribed handler from a channel. */
	UFUNCTION(BlueprintCallable, Category = "UI|Events")
	void Unsubscribe(FGameplayTag Channel, const FUIEventHandler& Handler);

	/** Remove every handler an object registered across all channels (call on teardown). */
	UFUNCTION(BlueprintCallable, Category = "UI|Events")
	void UnsubscribeAll(UObject* Subscriber);

	/** Broadcast a payload to every subscriber of a channel. */
	UFUNCTION(BlueprintCallable, Category = "UI|Events")
	void Broadcast(FGameplayTag Channel, UObject* Payload);

	// USubsystem
	virtual void Deinitialize() override;

private:
	/** Channel -> subscribed handlers. */
	UPROPERTY()
	TMap<FGameplayTag, FUIEventMulticast> Channels;
};
