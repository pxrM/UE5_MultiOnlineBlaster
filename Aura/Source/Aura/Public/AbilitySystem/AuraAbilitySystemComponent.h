// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);

/**
 * 
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
	void EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	
public:
	// 获取到一个ge资产后触发的委托
	FEffectAssetTags EffectAssetTags;
	
};
