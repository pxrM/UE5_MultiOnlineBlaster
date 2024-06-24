// P


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(const ECharacterClassType CharacterClass)
{
	return CharacterClassInformation.FindChecked(CharacterClass);
}
