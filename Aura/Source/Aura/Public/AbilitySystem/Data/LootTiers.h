// P

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTiers.generated.h"


// 物品的掉落内容和概率
USTRUCT(BlueprintType)
struct FLootItem
{
	GENERATED_BODY()

	// 在场景中的显示actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LootTiers|Spawning")
	TSubclassOf<AActor> LootClass;

	// 掉落概率
	UPROPERTY(EditAnywhere, Category="LootTiers|Spawning")
	float ChanceToSpawn = 0.f;

	// 最大掉落数量
	UPROPERTY(EditAnywhere, Category="LootTiers|Spawning")
	int32 MaxNumberToSpawn = 0;

	// 是否修改物品生成等级，false则使用敌人等级
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LootTiers|Spawning")
	bool bLootLevelOverride = true;
};


/**
 * 敌人被击败后，掉落战利品的功能。定义掉落体的内容
 */
UCLASS()
class AURA_API ULootTiers : public UDataAsset
{
	GENERATED_BODY()

public:
	// 获取需要掉落的数据
	UFUNCTION(BlueprintCallable)
	TArray<FLootItem> GetLootItems();

public:
	UPROPERTY(EditDefaultsOnly, Category="LootTiers|Spawning")
	TArray<FLootItem> LootItems;
};
