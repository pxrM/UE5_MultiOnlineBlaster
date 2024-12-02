// P


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "AbilitySystem/AuraAttributeSet.h"

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

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	// 获取冷却ge
	if(const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		// 遍历ge修改的内容
		for(FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if(Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				// 通过修饰符获取到使用的FScalableFloat并计算传入等级的蓝量消耗，FScalableFloat是受保护性的属性，无法直接获取，只能通过函数
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if(const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
