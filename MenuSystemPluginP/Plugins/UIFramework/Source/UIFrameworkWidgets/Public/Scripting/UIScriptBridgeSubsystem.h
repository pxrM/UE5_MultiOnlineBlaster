// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "UILayerTypes.h"
#include "UILifecycleTypes.h"
#include "UIWidgetRegistry.h"
#include "UIScriptBridgeSubsystem.generated.h"

class UUserWidget;
class UUIManagerSubsystem;
class UUIScriptAsyncRequest;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FUIScriptWidgetOpenEvent, UUserWidget*, Widget, const FUIOpenContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FUIScriptWidgetCloseEvent, UUserWidget*, Widget, const FUICloseContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FUIScriptWidgetActivationEvent, UUserWidget*, Widget, FGameplayTag, Key, bool, bIsActive);

/**
 * Reflection-friendly facade over UUIManagerSubsystem. Script integrations bind
 * here instead of depending on layer/cache internals or a specific script runtime.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUIScriptBridgeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Scripting", meta = (WorldContext = "WorldContextObject"))
	static UUIScriptBridgeSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	UUserWidget* OpenUI(FGameplayTag Key, UObject* Payload = nullptr);

	/** Script-friendly asynchronous open with a retained, cancellable request object. */
	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	UUIScriptAsyncRequest* OpenUIAsync(FGameplayTag Key, UObject* Payload = nullptr);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	bool CloseUI(FGameplayTag Key, UObject* Result = nullptr);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	bool CloseWidget(UUserWidget* Widget, UObject* Result = nullptr);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	void ForceCloseAllUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	bool HandleBackAction();

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting")
	bool ValidateConfiguration(UPARAM(ref) TArray<FString>& OutErrors) const;

	/** String-friendly runtime registration used by Lua/JS table adapters. */
	UFUNCTION(BlueprintCallable, Category = "UI|Scripting|Registry")
	bool RegisterRuntimeWidget(
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
		FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting|Registry")
	bool UnregisterRuntimeWidget(const FString& KeyName, FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "UI|Scripting|Registry")
	void ClearRuntimeWidgets();

	UFUNCTION(BlueprintPure, Category = "UI|Scripting|Registry")
	int32 GetRuntimeWidgetCount() const;

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptWidgetOpenEvent OnWidgetOpening;

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptWidgetOpenEvent OnWidgetOpened;

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptWidgetCloseEvent OnWidgetClosing;

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptWidgetCloseEvent OnWidgetClosed;

	UPROPERTY(BlueprintAssignable, Category = "UI|Scripting")
	FUIScriptWidgetActivationEvent OnWidgetActivationChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	friend class UUIScriptAsyncRequest;
	void CancelRequest(UUIScriptAsyncRequest* Request);
	void CompleteRequest(UUIScriptAsyncRequest* Request, UUserWidget* Widget);
	UUIManagerSubsystem* GetManager() const;
	void HandleWidgetOpening(UUserWidget* Widget, const FUIOpenContext& Context);
	void HandleWidgetOpened(UUserWidget* Widget, const FUIOpenContext& Context);
	void HandleWidgetClosing(UUserWidget* Widget, const FUICloseContext& Context);
	void HandleWidgetClosed(UUserWidget* Widget, const FUICloseContext& Context);
	void HandleWidgetActivationChanged(UUserWidget* Widget, FGameplayTag Key, bool bIsActive);

	UPROPERTY()
	TMap<int32, TObjectPtr<UUIScriptAsyncRequest>> ActiveRequests;

	int32 NextRequestId = 1;
};
