// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"


/*
 * 一个普通的GA的生命周期在Server-Client之间都会产生2-3个RPCs的调用。
 * 激活ge：CallServerTryActivateAbility()
 * 向server同步数据：ServerSetReplicatedTargetData() (可选)
 * 结束ge：ServerEndAbility()
 */


class ULoadScreenSaveGame;
/*
 *  获取到一个ge资产后触发的委托
 *	  FGameplayTagContainer：
 *		  多个GameplayTags可以使用FGameplayTagContainer来存储，相比于常规的TArray<FGameplayTag>，前者更加有效率(efficiency magic)
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);
/*
 * 技能初始化应用后的回调委托
 */
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven)
/*
 * 传递给asc，然后遍历所有的激活能力
 */
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&)
/*
 * ga的状态标签发生改变通知
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const int32 /*AbilityLevel*/);
/*
 * 技能装配后的委托
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);
/*
 * 被动技能结束通知
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*AbilityTag*/);
/*
 * 是否激活被动技能特效
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FActivePassiveEffect, const FGameplayTag& /*AbilityTag*/, const bool /*bActivate*/);



/**
 * gas组件
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 组件上的actor相关信息设置后调用
	void AbilityActorInfoSet();
	
	// 添加角色能力
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	// 添加角色被动能力
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
	// 根据本地存档添加角色能力
	void AddCharacterAbilitiesFromSaveData(const ULoadScreenSaveGame* SaveGame);

	// 触发技能时按键按下
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	// 触发技能的悬停时触发
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	// 触发技能时按键离开
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	// 遍历技能，并通过委托回调的形式广播出去
	void ForEachAbility(const FForEachAbility& Delegate);
	
	// 通过传入的技能实例，从技能实例里面获取到对应的技能标签和输入标签
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetAbilityInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetAbilityStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetAbilityStatusTagFromAbilityTag(const FGameplayTag& AbilityTag);
	FGameplayTag GetAbilityInputTagFromAbilityTag(const FGameplayTag& AbilityTag);
	bool SlotIsEmpty(const FGameplayTag& Slot);
	static bool AbilityHasSlot(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& Slot);
	static bool AbilityHasAnySlot(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);
	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec) const;
	static void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	// 升级对应的属性(本地)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	// 升级对应的属性(server)
	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	// 通过标签获取角色身上的技能实例
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	// 根据角色等级更新技能状态
	void UpdateAbilityStatus(const int32 Level);

	// 花费角色技能点（升级技能）
	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);

	// 在服务器处理技能装配，传入技能标签和装配插槽对应的输入标签
	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);
	// 处理客户端的技能装配
	UFUNCTION(Client, Reliable)
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
	// 清除技能装配插槽的技能
	static void ClearSlot(FGameplayAbilitySpec* Spec);
	// 根据输入标签，清除技能装配插槽的技能
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);
	// 判断当前技能实例是否处于目标技能装配插槽
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
	// 获取技能当前等级描述和下一等级描述 @return 技能是否解锁
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);
	// 网络广播被动技能特效是否开启
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastActivatePassiveEffect(const FGameplayTag& AbilityTag, bool bActivate);
	

protected:
	virtual void OnRep_ActivateAbilities() override;
	
	// 用于在Effect应用到自身角色时触发相应的逻辑
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
	// ga状态改变后同步到客户端
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel);
	
	
public:
	// 获取到一个ge资产后触发的委托
	FEffectAssetTags EffectAssetTags;
	// 技能初始化后的广播委托
	FAbilitiesGiven AbilitiesGivenDelegate;
	// 技能状态改变后广播
	FAbilityStatusChanged AbilityStatusChanged;
	// 技能装备后广播
	FAbilityEquipped AbilityEquippedDelegate;
	// 被动技能失效广播
	FDeactivatePassiveAbility DeactivatePassiveAbility;
	// 激活或者关闭被动技能特效广播
	FActivePassiveEffect ActivePassiveEffect;
	// 初始化应用技能后设置为true，记录当前是否初始化完成
	bool bStartupAbilitiesGiven = false;
};
