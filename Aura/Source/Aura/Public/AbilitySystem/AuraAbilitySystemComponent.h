// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"


/*
 * 一个普通的GA的生命周期在Server-Client之间都会产生2-3个RPCs的调用。
 * 激活ge：CallServerTryActivateAbility()
 * 向server同步数据：ServerSetReplicatedTargetData() (可选)
 * 结束ge：ServerEndAbility()
 */


/*
 *  获取到一个ge资产后触发的委托
 *	  FGameplayTagContainer：
 *		  多个GameplayTags可以使用FGameplayTagContainer来存储，相比于常规的TArray<FGameplayTag>，前者更加有效率(efficiency magic)
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);

/*
 * 技能初始化应用后的回调委托
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitiesGiven, UAuraAbilitySystemComponent*)


/**
 * gas组件
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 组件上的actor相关信息设置后调用
	void AbilityActorInfoSet();
	
	// 添加角色能力
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	// 触发技能的悬停时触发
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	// 触发技能时按键离开
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	
protected:
	// 用于在Effect应用到自身角色时触发相应的逻辑
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	
public:
	// 获取到一个ge资产后触发的委托
	FEffectAssetTags EffectAssetTags;
	// 技能初始化后的广播委托
	FAbilitiesGiven AbilitiesGivenDelegate;
	// 初始化应用技能后设置为true，记录当前是否初始化完成
	bool bStartupAbilitiesGiven = false;
};
