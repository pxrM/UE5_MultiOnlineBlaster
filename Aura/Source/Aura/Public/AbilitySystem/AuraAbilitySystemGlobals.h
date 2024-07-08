// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * 使用自定义Asset Manager需要配置：
 *	Config\DefaultGame.ini  =》 [/Script/GameplayAbilities.AbilitySystemGlobals]  =》 +AbilitySystemGlobalsClassName="/Script/Aura.AuraAbilitySystemGlobals"
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	/**
	 *	要使用自定义的上下文类，需要找到创建的位置，比如当前火球术使用的技能类里面，会去创建它通过ASC的MakeEffectContext()去创建的
	 *	在UAbilitySystemComponent::MakeEffectContext()函数里是通过UAbilitySystemGlobals::Get().AllocGameplayEffectContext()实现的创建
	 *	所以要创建 FAuraGameplayEffectContent 需要自定义一个 AbilitySystemGlobals，然后覆写这个函数。
	 */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
