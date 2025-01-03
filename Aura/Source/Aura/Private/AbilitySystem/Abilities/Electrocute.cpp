// P


#include "AbilitySystem/Abilities/Electrocute.h"

#include "AuraGameplayTags.h"

FString UElectrocute::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Lightning);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(
			TEXT(
				"<Title>电击</>\n\n"
				
				"<Small>Level: </>"
				"<Level>%d</>\n"
				
				"<Small>蓝量消耗: </>"
				"<ManaCost>%.1f</>\n\n"
				
				"<Small>技能冷却: </>"
				"<Cooldown>%.1f</>\n\n"
				
				"<Default>发射一条闪电造成 </>"
				"<Damage>%d</><Default>点伤害。</>"),
			Level, ManaCost, Cooldown, Damage);
	}
	else
	{
		return FString::Printf(
			TEXT(
				"<Title>电击</>\n\n"

				"<Small>Level: </>"
				"<Level>%d</>\n"

				"<Small>蓝量消耗: </>"
				"<ManaCost>%.1f</>\n\n"

				"<Small>技能冷却: </>"
				"<Cooldown>%.1f</>\n\n"

				"<Default>发射一条闪电，并在命中敌人后，向外扩散 %i 条闪电链，造成 </>"
				"<Damage>%d</>"
				"<Default>点雷电伤害，在技能结束时对敌人造成</><Time>%.1f</><Default>秒的眩晕效果。</>"),
			Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShockTarget - 1), Damage, DeBuffDuration);
	}
}

FString UElectrocute::GetNextDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Lightning);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(
			TEXT(
				"<Title>下一等级</>\n\n"

				"<Small>Level: </>"
				"<Level>%d</>\n"

				"<Small>蓝量消耗: </>"
				"<ManaCost>%.1f</>\n\n"

				"<Small>技能冷却: </>"
				"<Cooldown>%.1f</>\n\n"

				"<Default>发射一条闪电，并在命中敌人后，向外扩散 %i 条闪电链，造成 </>"
				"<Damage>%d</>"
				"<Default>点雷电伤害，在技能结束时对敌人造成</><Time>%.1f</><Default>秒的眩晕效果。</>"),
			Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShockTarget - 1), Damage, DeBuffDuration);
}
