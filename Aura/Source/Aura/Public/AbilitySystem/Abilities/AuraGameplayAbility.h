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
	virtual FString GetDescription(int32 Level);
	virtual FString GetNextDescription(int32 Level);
	static  FString GetLockedDescription(int32 Level);

public:
	// 启动输入能力的tag
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

protected:
	// 获取技能蓝量消耗
	float GetManaCost(float InLevel = 1.f) const;
	// 获取技能冷却时间
	float GetCooldown(float InLevel = 1.f) const;
};
