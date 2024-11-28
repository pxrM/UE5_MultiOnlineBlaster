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

	GetAuraPS()->OnSpellPointChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointChangedSignature.Broadcast(SpellPoints);
		
		CurrentSpellPoints = SpellPoints;
		BroadSelectedSpellGlobeData();
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
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

void USpellMenuWidgetController::SpendPointBtnPressed()
{
	if(GetAuraASC())
	{
		GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,
                                                     bool& bShouldEnableSpellPointsBtn, bool& bShouldEnableEquipBtn)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const bool bHasSpellPoint = SpellPoints > 0;

	if(AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
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
	ShouldEnableButtons(SelectedAbility.AbilityTag, CurrentSpellPoints, bEnableSpendPoint, bEnableEquip);
	
	FString Description;
	FString NextLevelDescription;
	GetAuraASC()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag, Description, NextLevelDescription);
	
	SpellGlobeSelectedSignature.Broadcast(bEnableSpendPoint, bEnableEquip, Description, NextLevelDescription);
}
