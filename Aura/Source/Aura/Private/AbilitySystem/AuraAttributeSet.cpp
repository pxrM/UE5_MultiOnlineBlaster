// P


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AuraLogChannels.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	/* Primary */
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);

	/* Secondary */
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);

	/* Resistance */
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Health: %f"), NewValue);
		NewValue = FMath::Clamp(NewValue, 0, GetMaxHealth());
	}

	if (Attribute == GetManaAttribute())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Mana: %f"), NewValue);
		NewValue = FMath::Clamp(NewValue, 0, GetMaxMana());
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Mana from GetHealth(): %f"), GetHealth());
	// 	UE_LOG(LogTemp, Warning, TEXT("Magnitude 差异幅度: %f"), Data.EvaluatedData.Magnitude);
	// }

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	// 判断当前目标是否已经死亡
	if(Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter))
		return;

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
		// UE_LOG(LogTemp, Warning, TEXT("Changed Health on %s, Health: %f"), *Props.TargetAvatarActor->GetName(),GetHealth());
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0, GetMaxMana()));
	}

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}

	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props);
	}
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage((0.f));
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		// 是否死亡 
		if (const bool bFatal = NewHealth <= 0.f)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
			{
				CombatInterface->Die(UAuraAbilitySystemLibrary::GetDeathImpulse(Props.EffectContextHandle));
			}
			SendXPEvent(Props);
		}
		else
		{
			// 受到雷电持续攻击时不会激活
			if(Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter))
			{
				// 根据标签激活受击能力
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effect_HitReact);
				Props.TargetAsc->TryActivateAbilitiesByTag(TagContainer);
			}
			
			// 判断是否击退
			const FVector& KnockbackForce = UAuraAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);
			if(!KnockbackForce.IsNearlyZero(1.f))
			{
				/*
				 * 将为角色设置一个待处理的发射速度 (LaunchVelocity)，并在角色CharacterMovementComponent下一次更新时应用这个速度。
				 * 角色会被设置为“falling”（下落）状态，并触发 OnLaunched 事件。这通常用于角色跳跃、被抛出或其他瞬时位移的情况。
				 *		LaunchVelocity: 这是一个 FVector 类型的参数，表示施加给角色的速度。
				 *		bXYOverride: 如果设置为 true，将替换角色当前的 X 和 Y 方向速度，而不是在现有速度的基础上添加。
				 *					 这意味着角色在 X 和 Y 方向上的速度会被直接设置为 LaunchVelocity 中的对应值。
				 *		bZOverride: 如果设置为 true，将替换角色当前的 Z 方向速度（通常与垂直跳跃有关），而不是在现有速度的基础上添加。
				 *					角色在 Z 方向的速度会被设置为 LaunchVelocity 中的 Z 值。
				 */
				Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			}
		}

		const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		const bool bCritical = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
										 FString::Printf(TEXT("LocalIncomingDamage: %f"), LocalIncomingDamage));
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCritical);

		// 处理debuff
		if(UAuraAbilitySystemLibrary::IsSuccessfulDeBuff(Props.EffectContextHandle))
		{
			HandleDeBuff(Props);
		}
	}
}

void UAuraAttributeSet::HandleDeBuff(const FEffectProperties& Props)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDeBuffDamageType(Props.EffectContextHandle);
	const FGameplayTag DebuffType = GameplayTags.DamageTypesToDeBuff[DamageType];
	const float DeBuffDamage = UAuraAbilitySystemLibrary::GetDeBuffDamage(Props.EffectContextHandle);
	const float DeBuffDuration = UAuraAbilitySystemLibrary::GetDeBuffDuration(Props.EffectContextHandle);
	const float DeBuffFrequency = UAuraAbilitySystemLibrary::GetDeBuffFrequency(Props.EffectContextHandle);

	// 创建一个新的 UGameplayEffect 对象，该对象不会被序列化并保存在磁盘上，因为它是在暂时性包（transient）中创建的。
	FString DeBuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DebuffType.ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DeBuffName));

	/*
	 * 设置动态创建的ge属性
	 * */
	// 设置ge的生命周期为有时间限制的
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	// 设置ge的持续时间
	Effect->DurationMagnitude = FScalableFloat(DeBuffDuration);
	// 设置ge的触发间隔时间
	Effect->Period = FScalableFloat(DeBuffFrequency);

	/*  ******** In 5.3.2 works only like this */
	// 添加向目标Actor增加对应的标签组件 
	FInheritedTagContainer TagContainer = FInheritedTagContainer();
	UTargetTagsGameplayEffectComponent& TargetTagsGameplayEffectCmp = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	// 添加debuff标签
	TagContainer.Added.AddTag(DebuffType);
	TagContainer.CombinedTags.AddTag(DebuffType);
	// 如果是眩晕debuff添加禁用输入标签
	if(DebuffType.MatchesTagExact(GameplayTags.DeBuff_Stun))
	{
		TagContainer.Added.AddTag(GameplayTags.Player_Block_CursorTrace);
		TagContainer.Added.AddTag(GameplayTags.Player_Block_InputHeld);
		TagContainer.Added.AddTag(GameplayTags.Player_Block_InputPressed);
		TagContainer.Added.AddTag(GameplayTags.Player_Block_InputReleased);
	}
	// 应用并更新标签容器
	TargetTagsGameplayEffectCmp.SetAndApplyTargetTagChanges(TagContainer);
	/*  ******** In 5.3.2 works only like this  */
	
	// 应用一个ge时，如果这个ge可以被堆叠，需要指定堆叠的方式。AggregateBySource意味着堆叠是基于应用这个GE的源头（比如攻击者、技能等）进行的。
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	// 设置堆叠限制层数
	Effect->StackLimitCount = 1;

	// 获取当前修改属性的Modifiers的长度，也就是下一个添加的modify的下标索引
	const int32 Index = Effect->Modifiers.Num();
	// 添加一个新的modify
	Effect->Modifiers.Add(FGameplayModifierInfo());
	// 通过索引获取这个modify
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];
	// 设置要应用的数值
	ModifierInfo.ModifierMagnitude = FScalableFloat(DeBuffDamage);
	// 设置应用的的运算符号为+
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	// 设置要应用修改的属性为造成伤害
	ModifierInfo.Attribute = GetIncomingDamageAttribute();

	/*
	 * 创建ge实例
	 */
	FGameplayEffectContextHandle EffectContextHandle = Props.SourceAsc->MakeEffectContext();
	EffectContextHandle.AddSourceObject(Props.SourceCharacter);
	if(const FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContextHandle, 1.f))
	{
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
		const TSharedPtr<FGameplayTag> DeBuffDamageType = MakeShareable(new FGameplayTag(DamageType));
		AuraContext->SetDeBuffDamageType(MakeShareable(new FGameplayTag(DamageType)));
		// 应用给目标
		Props.TargetAsc->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0);
	// UE_LOG(LogAura, Log, TEXT("Incoming XP: %f"), LocalIncomingXP);
		
	if(Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		// 1. 检查是否可以升级
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		if(NumLevelUps > 0)
		{
			/*
			 * 升级
			 */
			const int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointReward(Props.SourceCharacter, CurrentLevel);
			const int32 SpellPointsReward = IPlayerInterface::Execute_GetSpellPointReward(Props.SourceCharacter, CurrentLevel);

			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);
			IPlayerInterface::Execute_AddToAttributePoint(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoint(Props.SourceCharacter, SpellPointsReward);
				
			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
				
			// SetHealth(GetMaxHealth());
			// SetMana(GetMaxMana());
			bTopOffHealth = true;
			bTopOffMana = true;
		}
		// 2. 将经验应用给自身，通过事件传递，在玩家角色被动技能GA_ListenForEvents里接收
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if(Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;
	}
	if(Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;
	}
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, const float Damage, const bool bBlockedHit,
                                         const bool bCriticalHit) const
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		// 从技能释放者身上获取PC然后显示HUD
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
		// 从受击目标身上获取PC然后显示HUD
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClassType TargetType = ICombatInterface::Execute_GetCharacterType(Props.TargetCharacter);
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetType, TargetLevel);

		// 发送事件给源角色（造成伤害的）
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
		Payload.EventMagnitude = XPReward;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* SourceAsc = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (IsValid(SourceAsc) && SourceAsc->AbilityActorInfo.IsValid() && SourceAsc->AbilityActorInfo->AvatarActor.
		IsValid())
	{
		Props.SourceAvatarActor = SourceAsc->GetAvatarActor();
		Props.SourceController = SourceAsc->AbilityActorInfo->PlayerController.Get();
		Props.SourceAsc = SourceAsc;
		if (Props.SourceAvatarActor != nullptr && Props.SourceController == nullptr)
		{
			// 角色可以是各种类型的，其中包括可以由玩家控制的角色（通常是玩家控制的主要角色）和由游戏系统控制的非玩家角色（NPC）。这些角色可以被实现为不同的类，以便于管理和执行各种游戏逻辑。
			// 在某些情况下，游戏中的某个效果可能由非玩家角色触发，或者由AI自动执行，而不是由玩家直接操作。在这种情况下，这个角色可能没有直接关联的玩家控制器，因为它不是由玩家控制的。
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetAsc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UAuraAttributeSet::OnRep_OnStrength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_OnResilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_OnIntelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_OnVigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_OnArmor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_OnArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_OnBlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_OnCriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_OnCriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_OnCriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_OnHealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_OnManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_OnMaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_OnMaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_OnFireResistance(const FGameplayAttributeData& OldFireResistance) const
{
}

void UAuraAttributeSet::OnRep_OnLightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
}

void UAuraAttributeSet::OnRep_OnArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
}

void UAuraAttributeSet::OnRep_OnPhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
}

void UAuraAttributeSet::OnRep_OnHealth(const FGameplayAttributeData& OldHealth) const
{
	// 用来告诉ASC记录下服务器正在修改这个参数记录它的旧值和注册它新的值，方便有问题时数据回滚等等行为。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_OnMana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}
