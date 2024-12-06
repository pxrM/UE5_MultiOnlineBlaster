// P


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfoData.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitValues()
{
	BroadcastAbilityInfo();
	SpellPointChangedSignature.Broadcast(GetAuraPS()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
	{
		if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.StatusTag = StatusTag;
			BroadSelectedSpellGlobeData();
		}
		FAuraAbilityInfo Info = AbilityDataTable->FindAbilityInfoForTag(AbilityTag);
		Info.StatusTag = StatusTag;
		AbilityInfoDelegate.Broadcast(Info);
	});

	GetAuraASC()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	GetAuraPS()->OnSpellPointChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointChangedSignature.Broadcast(SpellPoints);
		
		CurrentSpellPoints = SpellPoints;
		BroadSelectedSpellGlobeData();
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if(bWaitingForEquipSelection)
	{
		const FGameplayTag AbilityType = AbilityDataTable->FindAbilityInfoForTag(AbilityTag).AbilityTypeTag;
		StopWaitingForEquipSignature.Broadcast(AbilityType);
		bWaitingForEquipSelection = false;
	}
	
	FGameplayTag AbilityStatus;
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	CurrentSpellPoints = GetAuraPS()->GetSpellPoints();
     
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if(!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}
	else
	{
		AbilityStatus = GetAuraASC()->GetAbilityStatusTagFromSpec(*AbilitySpec);
	}

	SelectedAbility.AbilityTag = AbilityTag;
	SelectedAbility.StatusTag = AbilityStatus;
	BroadSelectedSpellGlobeData();
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if(bWaitingForEquipSelection)
	{
		const FGameplayTag AbilityType = AbilityDataTable->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
		StopWaitingForEquipSignature.Broadcast(AbilityType);
		bWaitingForEquipSelection = true;
	}
	
	SelectedAbility.AbilityTag = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;
	SpellGlobeSelectedSignature.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::SpendPointBtnPressed()
{
	if(GetAuraASC())
	{
		GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::EquipBtnPressed()
{
	const FGameplayTag AbilityType = AbilityDataTable->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
	WaitForEquipSelectionSignature.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	const FGameplayTag SelectedStatus = GetAuraASC()->GetAbilityStatusTagFromAbilityTag(SelectedAbility.AbilityTag);
	if(SelectedStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetAuraASC()->GetAbilityInputTagFromAbilityTag(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityTypeTag)
{
	if (!bWaitingForEquipSelection)return;

	const FGameplayTag& SelectedAbilityType = AbilityDataTable->FindAbilityInfoForTag(SelectedAbility.AbilityTag).
	                                                            AbilityTypeTag;
	if (!SelectedAbilityType.MatchesTagExact(AbilityTypeTag)) return;

	GetAuraASC()->ServerEquipAbility(SelectedAbility.AbilityTag, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
                                                   const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	// 清除旧插槽的数据
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);
	// 更新新插槽的数据
	FAuraAbilityInfo SlotInfo = AbilityDataTable->FindAbilityInfoForTag(AbilityTag);
	SlotInfo.StatusTag = Status;
	SlotInfo.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(SlotInfo);
	
	StopWaitingForEquipSignature.Broadcast(SlotInfo.AbilityTypeTag);
	
	SpellGlobeReassignedSignature.Broadcast(AbilityTag);
	GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,
                                                     bool& bShouldEnableSpellPointsBtn, bool& bShouldEnableEquipBtn)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const bool bHasSpellPoint = SpellPoints > 0;

	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableSpellPointsBtn = bHasSpellPoint;
		bShouldEnableEquipBtn = true;
	}
	else if(AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		bShouldEnableSpellPointsBtn = bHasSpellPoint;
		bShouldEnableEquipBtn = false;
	}
	else if(AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableSpellPointsBtn = bHasSpellPoint;
		bShouldEnableEquipBtn = true;
	}
	else if(AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Locked))
	{
		bShouldEnableSpellPointsBtn = false;
		bShouldEnableEquipBtn = false;
	}
}

void USpellMenuWidgetController::BroadSelectedSpellGlobeData()
{
	bool bEnableSpendPoint = false;
	bool bEnableEquip = false;
	ShouldEnableButtons(SelectedAbility.StatusTag, CurrentSpellPoints, bEnableSpendPoint, bEnableEquip);
	
	FString Description;
	FString NextLevelDescription;
	GetAuraASC()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag, Description, NextLevelDescription);
	
	SpellGlobeSelectedSignature.Broadcast(bEnableSpendPoint, bEnableEquip, Description, NextLevelDescription);
}
