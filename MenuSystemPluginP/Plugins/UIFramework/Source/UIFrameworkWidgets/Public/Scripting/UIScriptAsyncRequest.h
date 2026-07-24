// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "UIScriptAsyncRequest.generated.h"

class UUserWidget;
class UUIScriptBridgeSubsystem;

UENUM(BlueprintType)
enum class EUIScriptRequestState : uint8
{
	Pending,
	Succeeded,
	Failed,
	Cancelled
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FUIScriptRequestCompleted, UUIScriptAsyncRequest*, Request, UUserWidget*, Widget, bool, bSucceeded);

/** GC-safe asynchronous UI request intended for Lua, TypeScript, and Blueprint. */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIScriptAsyncRequest : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "UI|Scripting")
	int32 GetRequestId() const { return RequestId; }

	UFUNCTION(BlueprintPure, Category = "UI|Scripting")
	FGameplayTag GetKey() const { return Key; }

	UFUNCTION(BlueprintPure, Category = "UI|Scripting")
	EUIScriptRequestState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "UI|Scripting")
	UUserWidget* GetWidget() const { return Widget; }

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptRequestCompleted OnCompleted;

private:
	friend class UUIScriptBridgeSubsystem;
	void InitializeRequest(UUIScriptBridgeSubsystem* InOwner, int32 InRequestId, FGameplayTag InKey);
	void Finish(UUserWidget* InWidget, EUIScriptRequestState InState);

	UFUNCTION()
	void HandleOpened(UUserWidget* InWidget);

	UPROPERTY()
	TObjectPtr<UUIScriptBridgeSubsystem> Owner;

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget;

	UPROPERTY()
	FGameplayTag Key;

	int32 RequestId = INDEX_NONE;
	EUIScriptRequestState State = EUIScriptRequestState::Pending;
};
