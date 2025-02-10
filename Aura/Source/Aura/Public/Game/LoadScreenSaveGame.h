// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"


struct FGameplayTag;
class UGameplayAbility;


// 当前存档可以显示umg类型的枚举
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant,
	EnterName,
	Taken,
};


// 保存actor相关信息的结构体
USTRUCT(BlueprintType)
struct FSavedActor
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorName = FName();

	UPROPERTY()
	FTransform Transform = FTransform();

	// Actor身上序列化变量的数据，必须通过UPROPERTY定义过，只在保存存档时使用。
	UPROPERTY()
	TArray<uint8> Bytes;
};
inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
	return Left.ActorName == Right.ActorName;
}


// 地图相关数据存储
USTRUCT(BlueprintType)
struct FSavedMap
{
	GENERATED_BODY()

	UPROPERTY()
	FString MapAssetName = FString();

	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};


// 保存技能相关信息的结构体
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()
	
	// 需要存储的技能
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;
	
	// 当前技能的标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();
	
	// 当前技能的状态标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag();
	
	// 当前技能的插槽标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot = FGameplayTag();
	
	// 当前技能的类型标签（被动还是主动技能）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag();

	// 当前技能等级
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel;

	// Array.h(971): Error C2678 : 二进制“==”: 没有找到接受“const FSavedAbility”类型的左操作数的运算符(或没有可接受的转换)
	bool operator==(const FSavedAbility& Other) const
	{
		return AbilityTag.MatchesTagExact(Other.AbilityTag);
	}
};


/**
 * USaveGame 是 UE 中的一个基类，用于在游戏中处理保存和加载数据。
 * 通过继承 USaveGame，ULoadScreenSaveGame 可以使用 Unreal 的保存系统来保存游戏状态、设置或其他数据。
 * USaveGame 提供的功能包括将对象的属性序列化到文件中，以及从文件中加载这些数据。这通常用于实现存档功能，保存玩家进度等。
 */
UCLASS()
class AURA_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	FSavedMap GetSavedMapWithMapName(const FString& InMapName);
	bool HasMap(const FString& InMapName);

public:
	// 存档名
	UPROPERTY()
	FString SlotName = FString();

	// 存档索引
	UPROPERTY()
	int32 SlotIndex = 0;

	// 存档的玩家名
	UPROPERTY()
	FString PlayerName = FString("Default Name");

	// 存档的地图名
	UPROPERTY()
	FString MapName = FString("Default Map Name");

	// 存档的默认玩家关卡出生位置的标签
	UPROPERTY()
	FName PlayerStartTag;

	// 进入存档界面时，默认显示的用户界面状态
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;
	
	// 是否是第一次加载
	UPROPERTY()
	bool bFirstTimeLoadIn = true;

	
	/*
	 * 增加保存玩家相关属性
	 */
	// 玩家等级
	UPROPERTY()
	int32 PlayerLevel = 1;
	
	// 经验值
	UPROPERTY()
	int32 XP = 0;

	// 可分配技能点
	UPROPERTY()
	int32 SpellPoints = 0;

	// 可分配属性点
	UPROPERTY()
	int32 AttributePoints = 0;
	
	// 力量
	UPROPERTY()
	float Strength = 0;

	// 智力
	UPROPERTY()
	float Intelligence = 0;

	// 韧性
	UPROPERTY()
	float Resilience = 0;

	// 体力
	UPROPERTY()
	float Vigor = 0;


	/*
	 *	技能相关 
	 */
	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;

	UPROPERTY()
	TArray<FSavedMap> SavedMaps;
};
