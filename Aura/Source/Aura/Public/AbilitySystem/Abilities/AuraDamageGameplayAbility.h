// P

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraDamageGameplayAbility.generated.h"

struct FTaggedMontage;

/**
 * 游戏伤害能力基类
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	// 添加伤害
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const;

	FDamageEffectParams MakeDamageEffectParamsFromClassDefault(AActor* TargetActor = nullptr);

protected:
	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType);

protected:
	// 该技能的伤害ge类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 在Map中设置多种伤害类型，并设置对应需造成的伤害值
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;

	// 触发减益效果概率
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeBuffChance = 20.f;
	// 减益效果伤害
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeBuffDamage = 5.f;
	// 减益伤害的触发时间间隔
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeBuffFrequency = 1.f;
	// 减益效果的持续时间
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeBuffDuration = 5.f;
	// 死亡时受到的冲击力
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeathImpulseMagnitude = 60.f;
};
