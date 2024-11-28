// P


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum", Level);
}

FString UAuraGameplayAbility::GetNextDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next level: </><Level>%d</> \n<Default>Causes much more demage.</>"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked Until level: %d</>"), Level);
}
