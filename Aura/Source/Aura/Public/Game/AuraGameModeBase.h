
// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "AuraGameModeBase.generated.h"

class ULoadScreenSaveGame;
class USaveGame;
class UMVVM_LoadSlot;
class UAbilityInfoData;
class UCharacterClassInfo;

/**
 * 
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	/**
	 * 保存新的存档
	 * @param LoadSlot 需要保存的视图实例
	 * @param SlotIndex 存档索引
	 */
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);

	/**
	 * 获取存档
	 * @param SlotName 存档名
	 * @param SlotIndex 存档索引
	 * @return 保存游戏的对象
	 */
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;

	/**
	 * 删除对应的存档
	 * @param SlotName 
	 * @param SlotIndex 
	 */
	static void DeleteSlotData(const FString& SlotName, const int32 SlotIndex);

	/**
	 * 地图传送
	 * @param Slot 对应的数据vm
	 */
	void TravelToMap(const UMVVM_LoadSlot* Slot);

	/**
	 * 获取当前游戏所使用的存档数据
	 * @return 
	 */
	ULoadScreenSaveGame* RetrieveInGameSaveData();

	/**
	 * 保存当前游戏进度
	 * @param SaveObject 
	 */
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject) const;

public:
	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category="Ability Info")
	TObjectPtr<UAbilityInfoData> AbilityInfo;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	// 初始地图名称
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;
	// 初始地图
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;
	// 默认玩家关卡出生位置的标签
	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;
	// 地图名和地图的映射。TSoftObjectPtr指针只保存路径，如果不使用，对应的资源不会加载到场景，可以在需要时再加载。
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;
	
};



