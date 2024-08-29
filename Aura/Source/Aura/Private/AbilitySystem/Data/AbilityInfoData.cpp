// P


#include "AbilitySystem/Data/AbilityInfoData.h"

#include "Aura/AuraLogChannels.h"

FAuraAbilityInfo UAbilityInfoData::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FAuraAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("Can't find info for AbilityTag [%s] on AbilityInfo [%s]"), *AbilityTag.ToString(),
		       *GetNameSafe(this));
	}

	return FAuraAbilityInfo();
}
