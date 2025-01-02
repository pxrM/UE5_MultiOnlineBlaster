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

	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefault(AActor* TargetActor = nullptr, FVector InRadialDamageOrigin = FVector::ZeroVector);

	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;

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
	float DeathImpulseMagnitude = 1000.f;
	// 技能命中时，敌人受到的击退力度
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackForceMagnitude = 500.f;
	// 击退概率
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackChance = 0.f;

	// 当前是否是范围伤害
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	bool bIsRadialDamage = false; 
	// 伤害中心点
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FVector RadialDamageOrigin = FVector::ZeroVector; 
	// 伤害内半径：在此半径内的目标会受到完整的伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageInnerRadius = 0.f; 
	// 伤害外半径：超过这个距离的目标受到最小伤害，最小伤害如果设置为0，则圈外不受到伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageOuterRadius = 0.f;
};
