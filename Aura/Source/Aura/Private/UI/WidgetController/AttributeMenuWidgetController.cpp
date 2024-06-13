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
		FAuraAttributeInfo Info = AttributeInfoData->FindAttributeInfoForTag(Pair.Key);
		Info.AttributeValue = Pair.Value().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{

}
