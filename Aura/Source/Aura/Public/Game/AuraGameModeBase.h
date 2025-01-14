// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

public:
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

public:
	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category="Ability Info")
	TObjectPtr<UAbilityInfoData> AbilityInfo;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;
	
};
