// P

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfoData.generated.h"


// 角色升级数据结构
USTRUCT()
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	// 该等级所需经验
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	// 该等级奖励的属性点
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;

	// 该等级奖励的技能点
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};


/**
 * 
 */
UCLASS()
class AURA_API ULevelUpInfoData : public UDataAsset
{
	GENERATED_BODY()

public:
	int32 FindLevelForXP(const int32 XP) const;

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraLevelUpInfo> LevelUpInformation;
};
