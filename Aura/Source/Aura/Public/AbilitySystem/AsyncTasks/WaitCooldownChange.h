// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"

struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 监听技能冷却
 * UBlueprintAsyncActionBase 提供了一种机制，允许创建能够在后台线程中异步执行的操作，并在操作完成后通知主线程。
 * meta=(ExposedAsyncProxy = "AsyncTask")
 *		元数据告诉引擎，将该类作为异步任务的代理进行处理。AsyncTask 是自定义的名称，代表这个类在蓝图中的异步操作代理。
 *		当在蓝图中调用这个类的异步方法时，UE会自动创建一个代理实例，并管理它的生命周期。
 *		允许将实例作为 AsyncTask 获取
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/*
	 * 宏标记函数为蓝图可调用，使其可以在蓝图脚本中使用。meta = (BlueprintInternalUseOnly = "true") 表示该函数仅用于蓝图内部，不应直接在游戏逻辑中调用。
	 * 静态工厂函数
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag);

	UFUNCTION(BlueprintCallable)
	void EndTask();


protected:
	// 监听冷却标签回调
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
	// 监听激活ge的回调
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
	
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	FGameplayTag CooldownTag;
	
public:
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;
	
	UPROPERTY(BlueprintAssignable) 
	FCooldownChangeSignature CooldownEnd;
};
