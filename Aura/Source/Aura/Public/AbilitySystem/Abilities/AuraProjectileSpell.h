// P

#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectileActor;

/**
 * 射弹技能
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

protected:
	/*
	 * 激活游戏能力（Gameplay Ability）
	 * Handle：游戏能力的句柄，用于唯一标识该游戏能力实例。
	 * ActorInfo：游戏角色信息，包括了激活能力的角色的相关数据和状态信息。
	 * ActivationInfo：激活信息，提供了有关激活游戏能力的详细信息，比如激活类型、原因等。
	 * TriggerEventData：激活事件的数据，包括了触发激活的具体事件信息。
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	// 生成子弹
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float OverridePitch = 0.f);

	
protected:
	// 子弹类
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAuraProjectileActor> ProjectileClass;
	// 子弹数
	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 5;
};
