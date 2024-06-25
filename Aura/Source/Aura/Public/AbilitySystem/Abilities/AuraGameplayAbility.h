// P

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * 能力：
 * 用于定义和实现游戏角色的各种能力（例如技能、攻击、法术等）。
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// 启动输入能力的tag
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

	// 在技能上通过技能等级设置伤害
	// FScalableFloat 是一种用于表示可缩放浮点数的结构体
	// 虽然不是所有的属性都会用到，但它占用的内存不大，所以直接写到基类，需要就用。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	FScalableFloat Damage;
};
