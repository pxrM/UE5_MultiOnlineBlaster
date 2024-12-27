// P

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 获取经验
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	// 获取奖励的属性点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointReward(const int32 Level) const;

	// 获取奖励的技能点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointReward(const int32 Level) const;

	// 向playerstate添加经验
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(const int32 InXP);

	// 获取属性点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePoints() const;

	// 添加属性点
	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoint(const int32 InAttributePoint);

	// 获取技能点
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPoint() const;
	
	// 添加技能点
	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoint(const int32 InSpellPoint);

	// 添加等级
	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(const int32 InPlayerLevel);

	// 根据经验获取等级
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(const int32 InXP) const;

	// 升级
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();

	// 显示贴花材质
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowMagicCircle(UMaterialInstance* DecalMaterial = nullptr);
	// 关闭贴花材质
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideMagicCircle();
};
