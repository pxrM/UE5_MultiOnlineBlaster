// P


#include "AbilitySystem/Data/LevelUpInfoData.h"

int32 ULevelUpInfoData::FindLevelForXP(int32 XP)
{
	int32 Level = 1;
	bool bSearching = true;
	
	while (bSearching)
	{
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if(XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}

	return Level;
}
