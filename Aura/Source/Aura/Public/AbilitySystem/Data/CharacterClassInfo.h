// P

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"


class UGameplayEffect;


// 角色类型
UENUM(BlueprintType)
enum class ECharacterClassType
{
	Elementalist, // 元素师
	Warrior,	  // 战士
	Ranger,		  // 游侠（漫游枪手）
};


// 角色默认信息结构体 
USTRUCT()
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	// 主要游戏技能
	UPROPERTY(EditDefaultsOnly, Category="Class Default")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
};



/**
 * 角色类型信息表
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FCharacterClassDefaultInfo GetClassDefaultInfo(const ECharacterClassType CharacterClass);

public:
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TMap<ECharacterClassType, FCharacterClassDefaultInfo> CharacterClassInformation;
	// 次要属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;
	// 重要属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;
};
