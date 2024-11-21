// P


#include "UI/WidgetController/SpellMenuWidgetController.h"

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
	GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
	{
		FAuraAbilityInfo Info = AbilityDataTable->FindAbilityInfoForTag(AbilityTag);
		Info.StatusTag = StatusTag;
		AbilityInfoDelegate.Broadcast(Info);
	});
	
	GetAuraPS()->OnSpellPointChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointChangedSignature.Broadcast(SpellPoints);
	});
}
