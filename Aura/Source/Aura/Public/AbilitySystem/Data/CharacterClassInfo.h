// P

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"


class UGameplayAbility;
class UGameplayEffect;


// 角色类型
UENUM(BlueprintType)
enum class ECharacterClassType
{
	Elementalist, // 元素师
	Warrior,	  // 战士
	Ranger,		  // 游侠(漫游枪手)
};


// 单个角色职业默认信息结构体 
USTRUCT()
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	// 主要游戏属性
	UPROPERTY(EditDefaultsOnly, Category="Class Default")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	// 职业的初始技能
	UPROPERTY(EditDefaultsOnly, Category="Class Default")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	// 经验奖励
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat XPReward = FScalableFloat();
};



/**
 * 游戏角色类型信息表
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FCharacterClassDefaultInfo GetClassDefaultInfo(const ECharacterClassType CharacterClass);

public:
	// 角色类型和角色信息对应的map
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TMap<ECharacterClassType, FCharacterClassDefaultInfo> CharacterClassInformation;

	//主要属性，玩家的基础属性，通过SetByCaller设置
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;
	
	// 次要属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;
	
	// 重要属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;
	
	// 初始能力
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	// 伤害计算系数表：
	// 在使用护甲穿透和护甲时，乘以一个数值来调整它们的影响。这是为了在RPG游戏中，前期游戏数值低，后期数值高，导致值的影响前期太低或者后期太高，
	// 所以，将其设置为一个可以跟随等级变动的数值，来让前期低数值时获取一个比例，并且在高等级时，不会导致数值溢出的问题。
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Damage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficients;
};
