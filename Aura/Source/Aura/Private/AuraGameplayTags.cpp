// P


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	/*
	 * Primary Tag
	 */
	GameplayTags.Attributes_Primary_Strength = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString("Increases physical damage")
	);
	GameplayTags.Attributes_Primary_Resilience = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increases Armor and Armor Penetration")
	);
	GameplayTags.Attributes_Primary_Intelligence = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increases magical damage")
	);
	GameplayTags.Attributes_Primary_Vigor = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increases Health")
	);
	/*
	 * Secondary Tag
	 */
	GameplayTags.Attributes_Secondary_Armor = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"),
		FString("Reduces damage taken, improves Block Chance")
	);
	GameplayTags.Attributes_Secondary_ArmorPenetration = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"),
		FString("Ignores Percentage of enemy Armor, increases Critical Hit Chance")
	);
	GameplayTags.Attributes_Secondary_BlockChance = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"),
		FString("Chance to cut incoming damage in half")
	);
	GameplayTags.Attributes_Secondary_CriticalHitChance = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double damage plus critical hit bonus")
	);
	GameplayTags.Attributes_Secondary_CriticalHitDamage = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus damage added when a critical hit is scored")
	);
	GameplayTags.Attributes_Secondary_CriticalHitResistance = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance"),
		FString("Reduces Critical Hit Chance of attacking enemies")
	);
	GameplayTags.Attributes_Secondary_HealthRegeneration = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of Health regenerated every 1 second")
	);
	GameplayTags.Attributes_Secondary_ManaRegeneration = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of Mana regenerated every 1 second")
	);
	GameplayTags.Attributes_Secondary_MaxMana = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum amount of Health obtainable")
	);
	GameplayTags.Attributes_Secondary_MaxHealth = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum amount of Mana obtainable")
	);
	/*
	 * Input Tag
	 */
	GameplayTags.InputTag_LMB = TagsManager.AddNativeGameplayTag(
		FName("InputTag.LMB"),
			FString("Input Tag for Left Mouse Button")
		);
	GameplayTags.InputTag_RMB = TagsManager.AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Right Mouse Button")
	);
	GameplayTags.InputTag_1 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.1"),
		FString("Input Tag for 1 key")
	);
	GameplayTags.InputTag_2 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.2"),
		FString("Input Tag for 2 key")
	);
	GameplayTags.InputTag_3 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.3"),
		FString("Input Tag for 3 key")
	);
	GameplayTags.InputTag_4 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.4"),
		FString("Input Tag for 4 key")
	);
	/*
	 * Damage 伤害
	 * 在正常的RPG游戏中，都存在一个类别就是属性伤害，比如，有一个火属性的技能，
	 * 它造成的伤害就是火属性类型的，并且它还有可能有附加伤害，比如给予目标一个灼烧效果，每秒造成多少的火属性伤害。
	 * 目标角色会有一个火属性伤害抵抗，根据百分比减少伤害。技能伤害区分物理伤害和魔法伤害，有些技能能够造成两种伤害。  
	 */
	GameplayTags.Damage = TagsManager.AddNativeGameplayTag(
		FName("Damage"),
		FString("基础伤害")
	);
	GameplayTags.Damage_Fire = TagsManager.AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("火属性伤害")
	);
	GameplayTags.Damage_Lightning = TagsManager.AddNativeGameplayTag(
		FName("Damage.Lightning"),
		FString("雷属性伤害")
	);
	GameplayTags.Damage_Arcane = TagsManager.AddNativeGameplayTag(
		FName("Damage.Arcane"),
		FString("魔法伤害")
	);
	GameplayTags.Damage_Physical = TagsManager.AddNativeGameplayTag(
		FName("Damage.Physical"),
		FString("物理伤害")
	);
	/*
	 * 抗性
	 */
	GameplayTags.Attributes_Resistance_Fire = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Resistance.Fire"),
		FString("火属性抗性")
	);
	GameplayTags.Attributes_Resistance_Lightning = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Resistance.Lightning"),
		FString("雷属性抗性")
	);
	GameplayTags.Attributes_Resistance_Arcane = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Resistance.Arcane"),
		FString("魔法伤害抗性")
	);
	GameplayTags.Attributes_Resistance_Physical = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Resistance.Physical"),
		FString("物理伤害抗性")
	);
	// 将属性和抗性标签对应
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);
	/*
	 * 技能
	 */
	GameplayTags.Abilities_Attack = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Attack"),
		FString("攻击技能标签")
	);
	/*
	 * 表现
	 */
	GameplayTags.Effect_HitReact = TagsManager.AddNativeGameplayTag(
		FName("Effect.HitReact"),
		FString("反应效果(比如受到伤害)")
	);

	GameplayTags.TagsInitCompleteNotify.ExecuteIfBound();
}
