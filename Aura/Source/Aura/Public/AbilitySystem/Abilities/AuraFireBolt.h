// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/**
 * 火球咒语技能
 */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextDescription(int32 Level) override;

	// 生成多个子弹
	UFUNCTION(BlueprintCallable, Category = "Projectiles")
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
	                      bool bOverridePitch = false, float OverridePitch = 0.f, AActor* HomingTarget = nullptr);

protected:
	// 攻击角度
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	float ProjectilesSpread = 90.f;
	// 生成子弹的最大数量
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	int32 MaxNumProjectiles;
};
