// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Engine/TargetPoint.h"
#include "AuraEnemySpawnPoint.generated.h"

enum class ECharacterClassType;
class AAuraEnemy;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	// 生成敌人
	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

	// 需要生成的敌人蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	TSubclassOf<AAuraEnemy> EnemyClass;

	// 需要生成的敌人的等级
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	int32 EnemyLevel = 1;

	// 敌人的职业类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	ECharacterClassType CharacterClassType = ECharacterClassType::Warrior;
};
