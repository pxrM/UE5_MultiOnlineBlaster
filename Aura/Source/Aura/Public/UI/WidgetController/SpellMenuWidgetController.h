// P

#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayTags.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"


struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpellGlobeSelectedSignature, bool, bSpellPointBtnEnable, bool, bEquipBtnEnable);

// 本地缓存选中的技能
struct FSelectedAbility
{
	FGameplayTag AbilityTag = FGameplayTag();
	FGameplayTag StatusTag = FGameplayTag();
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitValues() override;
	virtual void BindCallbacksToDependencies() override;
	// 技能按钮选中调用函数，处理升级按钮和装配
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);
	// 技能点花费按钮调用（技能升级）
	UFUNCTION(BlueprintCallable)
	void SpendPointBtnPressed();

private:
	// 通过技能状态标签和可分配技能点数来判断该技能是否可以装配和是否可以升级
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsBtn, bool& bShouldEnableEquipBtn);

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature SpellPointChangedSignature;
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedSignature;

private:
	FSelectedAbility SelectedAbility = {FAuraGameplayTags::Get().Abilities_Type_None, FAuraGameplayTags::Get().Abilities_Status_Locked};
	int32 CurrentSpellPoints = 0;
};
