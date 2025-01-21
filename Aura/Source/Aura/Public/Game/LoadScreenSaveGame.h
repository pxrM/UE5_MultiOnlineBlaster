// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"


// 当前存档可以显示umg类型的枚举
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant,
	EnterName,
	Taken,
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
};
