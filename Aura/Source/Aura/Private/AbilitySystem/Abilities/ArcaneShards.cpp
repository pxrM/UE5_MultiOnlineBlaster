// P


#include "AbilitySystem/Abilities/ArcaneShards.h"

#include "AuraGameplayTags.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Arcane);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(
			TEXT(
				"<Title>法术爆发</>\n\n"

				"<Small>Level: </>"
				"<Level>%d</>\n"

				"<Small>蓝量消耗: </>"
				"<ManaCost>%.1f</>\n\n"

				"<Small>技能冷却: </>"
				"<Cooldown>%.1f</>\n\n"
				
				"<Default>首次激活技能使用魔法光圈划定攻击位置，再次释放在魔法光圈位置生成 %i 奥术碎片，攻击附近敌人，造成</> <Damage>%i</> <Default>点法术伤害。</>"),
			Level, ManaCost, Cooldown, FMath::Min(Level,  MaxNumShards), Damage);
}

FString UArcaneShards::GetNextDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Arcane);
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
				
				"<Default>首次激活技能使用魔法光圈划定攻击位置，再次释放在魔法光圈位置生成 %i 奥术碎片，攻击附近敌人，造成</> <Damage>%i</> <Default>点法术伤害。</>"),
			Level, ManaCost, Cooldown, FMath::Min(Level,  MaxNumShards), Damage);
}
