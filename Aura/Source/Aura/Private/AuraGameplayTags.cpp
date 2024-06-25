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
	 * Damage
	 */
	GameplayTags.Damage = TagsManager.AddNativeGameplayTag(
		FName("Damage"),
		FString("伤害")
	);
}
