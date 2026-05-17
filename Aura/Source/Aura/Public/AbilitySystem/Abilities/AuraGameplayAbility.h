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
	/// <summary>
	/// 启动输入该能力的tag
	/// </summary>
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

protected:
	/// <summary>
	/// 获取技能蓝量消耗
	/// </summary>
	/// <param name="InLevel">技能等级</param>
	/// <returns></returns>
	float GetManaCost(float InLevel = 1.f) const;
	/// <summary>
	/// 获取技能冷却时间
	/// </summary>
	/// <param name="InLevel">技能等级</param>
	/// <returns></returns>
	float GetCooldown(float InLevel = 1.f) const;
};
