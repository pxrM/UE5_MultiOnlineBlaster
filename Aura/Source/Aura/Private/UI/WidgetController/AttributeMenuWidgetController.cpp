// P


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfoData.h"

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
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);

	 for(auto& Pair : AS->TagsToAttributes)
	 {
	 	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
	 		[this, Pair, AS](const FOnAttributeChangeData& Data)
	 		{
	 			BroadcastAttributeInfo(Pair.Key, Pair.Value());
	 		}
	 	);
	 }
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
	const FGameplayAttribute& Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfoData->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
