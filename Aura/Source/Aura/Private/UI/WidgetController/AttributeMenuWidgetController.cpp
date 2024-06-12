// P


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfoData.h"

void UAttributeMenuWidgetController::BroadcastInitValues()
{
	check(AttributeInfoData);

	const UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);

	FAuraAttributeInfo Info = AttributeInfoData->FindAttributeInfoForTag(FAuraGameplayTags::Get().Attributes_Primary_Strength);
	Info.AttributeValue = AS->GetStrength();
	AttributeInfoDelegate.Broadcast(Info);
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{

}
