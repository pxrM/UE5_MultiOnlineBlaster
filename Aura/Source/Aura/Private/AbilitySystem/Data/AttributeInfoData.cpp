// P


#include "AbilitySystem/Data/AttributeInfoData.h"

#include "Aura/AuraLogChannels.h"

FAuraAttributeInfo UAttributeInfoData::FindAttributeInfoForTag(const FGameplayTag& AttributeTag,
                                                               bool bLogNotFound) const
{
	for (const FAuraAttributeInfo& Info : AttributeInformation)
	{
		if(Info.AttributeTag.MatchesTag(AttributeTag))
		{
			return Info;
		}
	} 
	
	if(bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]"), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	
	return FAuraAttributeInfo();
}
