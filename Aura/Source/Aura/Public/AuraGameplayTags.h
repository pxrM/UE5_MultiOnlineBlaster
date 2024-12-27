// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

DECLARE_DELEGATE(FTagsInitCompleteNotify)

/**
 * 
 */
struct FAuraGameplayTags
{
public:
   static const FAuraGameplayTags& Get() { return GameplayTags; }
   static void InitializeNativeGameplayTags();

public:
   FGameplayTag Attributes_Primary_Strength;
   FGameplayTag Attributes_Primary_Resilience;
   FGameplayTag Attributes_Primary_Intelligence;
   FGameplayTag Attributes_Primary_Vigor;
   
   FGameplayTag Attributes_Secondary_Armor;
   FGameplayTag Attributes_Secondary_ArmorPenetration;
   FGameplayTag Attributes_Secondary_BlockChance;
   FGameplayTag Attributes_Secondary_CriticalHitChance;
   FGameplayTag Attributes_Secondary_CriticalHitDamage;
   FGameplayTag Attributes_Secondary_CriticalHitResistance;
   FGameplayTag Attributes_Secondary_HealthRegeneration;
   FGameplayTag Attributes_Secondary_ManaRegeneration;
   FGameplayTag Attributes_Secondary_MaxMana;
   FGameplayTag Attributes_Secondary_MaxHealth;

   FGameplayTag Attributes_Meta_IncomingXP;

   FGameplayTag Attributes_Resistance_Fire;
   FGameplayTag Attributes_Resistance_Lightning;
   FGameplayTag Attributes_Resistance_Arcane;
   FGameplayTag Attributes_Resistance_Physical;

   FGameplayTag DeBuff_Arcane;
   FGameplayTag DeBuff_Burn;
   FGameplayTag DeBuff_Physical;
   FGameplayTag DeBuff_Stun;
   // 属性伤害标签对应属性减益标签
   TMap<FGameplayTag, FGameplayTag> DamageTypesToDeBuff;

   FGameplayTag DeBuff_Chance;
   FGameplayTag DeBuff_Damage;
   FGameplayTag DeBuff_Duration;
   FGameplayTag DeBuff_Frequency;

   FGameplayTag Damage;
   FGameplayTag Damage_Fire;
   FGameplayTag Damage_Lightning;
   FGameplayTag Damage_Arcane;
   FGameplayTag Damage_Physical;
   // 属性伤害标签对应属性抵抗标签
   TMap<FGameplayTag, FGameplayTag> DamageTypesToResistance;

   FGameplayTag InputTag_LMB;
   FGameplayTag InputTag_RMB;
   FGameplayTag InputTag_1;
   FGameplayTag InputTag_2;
   FGameplayTag InputTag_3;
   FGameplayTag InputTag_4;
   FGameplayTag InputTag_Passive_1;
   FGameplayTag InputTag_Passive_2;

   FGameplayTag Abilities_None;

   FGameplayTag Abilities_Attack;
   FGameplayTag Abilities_Summon;

   FGameplayTag Abilities_HitReact;

   FGameplayTag Abilities_Status_Locked;
   FGameplayTag Abilities_Status_Eligible;
   FGameplayTag Abilities_Status_Unlocked;
   FGameplayTag Abilities_Status_Equipped;

   FGameplayTag Abilities_Type_Offensive;
   FGameplayTag Abilities_Type_Passive;
   FGameplayTag Abilities_Type_None;
   
   FGameplayTag Abilities_Fire_FireBolt;
   FGameplayTag Abilities_Lightning_Electrocute;
   FGameplayTag Abilities_Arcane_ArcaneShards;

   FGameplayTag Abilities_Passive_HaloOfProtection;
   FGameplayTag Abilities_Passive_LifeSiphon;
   FGameplayTag Abilities_Passive_ManaSiphon;

   FGameplayTag Cooldown_Fire_FireBolt;

   FGameplayTag CombatSocket_Weapon;
   FGameplayTag CombatSocket_RightHand;
   FGameplayTag CombatSocket_LeftHand;
   FGameplayTag CombatSocket_Tail;

   FGameplayTag Montage_Attack1;
   FGameplayTag Montage_Attack2;
   FGameplayTag Montage_Attack3;
   FGameplayTag Montage_Attack4;
   
   FGameplayTag Effect_HitReact;

   FGameplayTag Player_Block_InputPressed;
   FGameplayTag Player_Block_InputHeld;
   FGameplayTag Player_Block_InputReleased;
   FGameplayTag Player_Block_CursorTrace;
   
public:
   FTagsInitCompleteNotify TagsInitCompleteNotify; 
   
private:
   static FAuraGameplayTags GameplayTags;
};
