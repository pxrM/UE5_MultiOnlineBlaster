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
	* 元属性
	*/
	GameplayTags.Attributes_Meta_IncomingXP = TagsManager.AddNativeGameplayTag(
		FName("Attributes.Meta.IncomingXP"),
		FString("经验元属性标签")
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
	GameplayTags.InputTag_Passive_1 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.Passive.1"),
		FString("被动技能1输入tag")
	);
	GameplayTags.InputTag_Passive_2 = TagsManager.AddNativeGameplayTag(
		FName("InputTag.Passive.2"),
		FString("被动技能2输入tag")
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
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Lightning,GameplayTags.Attributes_Resistance_Lightning);
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
	GameplayTags.DamageTypesToResistance.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);
	/*
	 * debuff
	*/
	GameplayTags.DeBuff_Arcane = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Arcane"),
		FString("魔法减益标签")
	);
	GameplayTags.DeBuff_Burn = TagsManager.AddNativeGameplayTag(
			FName("DeBuff.Burn"),
			FString("火属性燃烧减益标签")
	);
	GameplayTags.DeBuff_Stun = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Stun"),
		FString("雷属性眩晕减益标签")
	);
	GameplayTags.DeBuff_Physical = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Physical"),
		FString("物理属性流血减益标签")
	);
	GameplayTags.DamageTypesToDeBuff.Add(GameplayTags.Damage_Fire, GameplayTags.DeBuff_Burn);
	GameplayTags.DamageTypesToDeBuff.Add(GameplayTags.Damage_Lightning,GameplayTags.DeBuff_Stun);
	GameplayTags.DamageTypesToDeBuff.Add(GameplayTags.Damage_Arcane, GameplayTags.DeBuff_Arcane);
	GameplayTags.DamageTypesToDeBuff.Add(GameplayTags.Damage_Physical, GameplayTags.DeBuff_Physical);
	/*
	 * debuff config tag
	*/
	GameplayTags.DeBuff_Chance = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Chance"),
		FString("减益效果触发几率配置标签")
	);
	GameplayTags.DeBuff_Damage = TagsManager.AddNativeGameplayTag(
			FName("DeBuff.Damage"),
			FString("减益效果触发伤害配置标签")
	);
	GameplayTags.DeBuff_Duration = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Duration"),
		FString("减益效果触发间隔时间配置标签")
	);
	GameplayTags.DeBuff_Frequency = TagsManager.AddNativeGameplayTag(
		FName("DeBuff.Frequency"),
		FString("减益效果触发持续时间配置标签")
	);
	/*
	 * 技能
	 */
	GameplayTags.Abilities_None = TagsManager.AddNativeGameplayTag(
		FName("Abilities.None"),
		FString("无技能")
	);
	GameplayTags.Abilities_Attack = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Attack"),
		FString("攻击技能标签")
	);
	GameplayTags.Abilities_Summon = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Summon"),
		FString("召唤技能标签")
	);
	GameplayTags.Abilities_Fire_FireBolt = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Fire.FireBolt"),
		FString("火球术技能标签")
	);
	GameplayTags.Abilities_Lightning_Electrocute = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Lightning.Electrocute"),
		FString("雷击技能标签")
	);

	GameplayTags.Abilities_HitReact = TagsManager.AddNativeGameplayTag(
		FName("Abilities.HitReact"),
		FString("受击技能标签")
	);
	GameplayTags.Abilities_Status_Locked = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Status.Locked"),
		FString("技能锁定状态标签")
	);
	GameplayTags.Abilities_Status_Eligible = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Status.Eligible"),
		FString("技能可解锁状态标签")
	);
	GameplayTags.Abilities_Status_Unlocked = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Status.Unlocked"),
		FString("技能解锁状态标签")
	);
	GameplayTags.Abilities_Status_Equipped = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Status.Equipped"),
		FString("技能装备状态标签")
	);

	GameplayTags.Abilities_Type_None = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Type.None"),
		FString("None")
	);
	GameplayTags.Abilities_Type_Offensive = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Type.Offensive"),
		FString("主动技能")
	);
	GameplayTags.Abilities_Type_Passive = TagsManager.AddNativeGameplayTag(
		FName("Abilities.Type.Passive"),
		FString("被动技能")
	);
	/*
	 * 技能冷却
	 */
	GameplayTags.Cooldown_Fire_FireBolt = TagsManager.AddNativeGameplayTag(
		FName("Cooldown.Fire.FireBolt"),
		FString("火球术技能冷却标签")
	);
	/*
	 * MeshSocket
	 */
	GameplayTags.CombatSocket_Weapon = TagsManager.AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),
		FString("使用武器部位攻击标签")
	);
	GameplayTags.CombatSocket_RightHand = TagsManager.AddNativeGameplayTag(
		FName("CombatSocket.RightHand"),
		FString("使用右手部位攻击标签")
	);
	GameplayTags.CombatSocket_LeftHand = TagsManager.AddNativeGameplayTag(
		FName("CombatSocket.LeftHand"),
		FString("使用左手部位攻击标签")
	);
	GameplayTags.CombatSocket_Tail = TagsManager.AddNativeGameplayTag(
		FName("CombatSocket.Tail"),
		FString("使用尾巴部位攻击标签")
	);
	/*
	 * 用来识别使用攻击动作的蒙太奇索引
	 */
	GameplayTags.Montage_Attack1 = TagsManager.AddNativeGameplayTag(
		FName("Montage.Attack.1"),
		FString("第1段蒙太奇攻击的标签")
	);
	GameplayTags.Montage_Attack2 = TagsManager.AddNativeGameplayTag(
		FName("Montage.Attack.2"),
		FString("第2段蒙太奇攻击的标签")
	);
	GameplayTags.Montage_Attack3 = TagsManager.AddNativeGameplayTag(
		FName("Montage.Attack.3"),
		FString("第3段蒙太奇攻击的标签")
	);
	GameplayTags.Montage_Attack4 = TagsManager.AddNativeGameplayTag(
		FName("Montage.Attack.4"),
		FString("第4段蒙太奇攻击的标签")
	);
	/*
	 * 表现
	 */
	GameplayTags.Effect_HitReact = TagsManager.AddNativeGameplayTag(
		FName("Effect.HitReact"),
		FString("反应效果(比如受到伤害)")
	);
	/*
	 * player tags
	 */
	GameplayTags.Player_Block_CursorTrace = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.CursorTrace"),
		FString("阻挡鼠标追踪")
		);
	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"),
		FString("阻挡按键按下")
		);
	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"),
		FString("阻挡按键抬起")
		);
	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"),
		FString("阻挡按键持续按下中")
		);

	GameplayTags.TagsInitCompleteNotify.ExecuteIfBound();
}
