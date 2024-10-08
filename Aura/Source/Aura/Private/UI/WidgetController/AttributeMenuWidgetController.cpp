// P


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfoData.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BroadcastInitValues()
{
	check(AttributeInfoData);

	const UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);

	// FAuraAttributeInfo StrengthInfo = AttributeInfoData->FindAttributeInfoForTag(FAuraGameplayTags::Get().Attributes_Primary_Strength);
	// StrengthInfo.AttributeValue = AS->GetStrength();
	// AttributeInfoDelegate.Broadcast(StrengthInfo);
	
	for(auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	const AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AttributePointChangedDelegate.Broadcast(AuraPlayerState->GetAttributePoints());
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);
	check(AS);
	for (auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair, AS](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnAttributePointChangedDelegate.AddLambda([this](int32 Points)
	{
		AttributePointChangedDelegate.Broadcast(Points);
	});
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
                                                            const FGameplayAttribute& Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfoData->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}
