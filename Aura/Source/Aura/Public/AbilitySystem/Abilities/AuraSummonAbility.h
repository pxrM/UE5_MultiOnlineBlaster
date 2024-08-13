// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 召唤能力
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	// 召唤的数量
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	int32 NumMinions = 5;
	// 召唤的角色类
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;
	// 召唤物距离召唤师最近的距离
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MinSpawnDistance = 50.f;
	// 召唤物距离召唤师最远的距离
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MaxSpawnDistance = 250.f;
	// 召唤物在召唤师前面的角度范围
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float SpawnSpread = 90.f;

	
public:
	// 获取生成的位置
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();
	// 随机一个要生成的actor类
	UFUNCTION(BlueprintPure, Category="Summoning")
	TSubclassOf<APawn> GetRandomMinionClass();
};
