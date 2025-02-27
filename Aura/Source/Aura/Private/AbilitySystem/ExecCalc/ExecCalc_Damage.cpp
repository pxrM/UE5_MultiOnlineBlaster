// P


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

// 创建一个结构用来保存所有需要捕获的属性(发起对象和目标对象)
struct AuraDamageStatics
{
	/* 声明对目标或自己的属性获取 start */
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	/* 声明对目标或自己的属性获取 end */
	
	AuraDamageStatics()
	{
		// 指定的类 S 中执行属性捕获的定义操作，将捕获的属性和相关定义存储起来。
		// 参数：S.属性集 P.属性名 T.目标还是自身 B.是否设置快照（true为创建时获取属性，false为应用时获取属性）
		// 这里获取 受击者 身上的属性值
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);
		// 这里获取 攻击者 身上的属性值
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatic;
	return DStatic;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	// 添加到捕获列表里
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);

	FAuraGameplayTags& AuraTags = const_cast<FAuraGameplayTags&>(FAuraGameplayTags::Get());
	AuraTags.TagsInitCompleteNotify.BindUObject(this, &UExecCalc_Damage::InitTagsToCaptureDefs);
}

void UExecCalc_Damage::InitTagsToCaptureDefs()
{
	// 添加标签和属性快照对应的数据
	const FAuraGameplayTags& AuraTags = FAuraGameplayTags::Get();
	TagsToCaptureDefs.Add(AuraTags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(AuraTags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(AuraTags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(AuraTags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (IsValid(SourceAvatar) && SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (IsValid(SourceAvatar) && SourceAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// 用于在 Aggregator（聚合器）中评估属性值时传递参数。聚合器在游戏中用于计算和管理属性（Attributes）的变化和影响，比如生命值、攻击力等
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// debuff
	DetermineDeBuff(ExecutionParams, Spec, EvaluateParameters);

	// 1. 根据标签获取由 Caller Magnitude 设置的伤害 
	float Damage = 0.f;
	for (auto& Pair : FAuraGameplayTags::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		checkf(TagsToCaptureDefs.Contains(ResistanceTag),
		       TEXT("TagsToCaptureDefs doesn't contain Tag:[%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		// 通过tag获取对应伤害值
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false); 
		// 获取抗性值
		float Resistance = 0.f;
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluateParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0, 100.f);
		/*
		 * 假设 DamageTypeValue 是 100（即原始伤害值），而 Resistance 是 20（即抵抗 20% 的伤害）：
		 *	计算步骤：
		 *		100.f - 20 = 80（即未抵抗的伤害占 80%）。
		 *		80 / 100.f = 0.8（转换为小数）。
		 *		DamageTypeValue *= 0.8 = 100 * 0.8 = 80。
		 *	因此，抵抗 20% 的伤害会使得最终伤害值变为 80。
		 */
		DamageTypeValue *= (100.f - Resistance) / 100.f;

		// 是否有范围伤害
		if(UAuraAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
		{
			// 1. 在Character里复写 TakeDamage
			// 2. 创建委托 OnDamageDelegate，广播在TakeDamage中收到的损害
			// 3. 绑定 lambda 到受害者的 OnDamageDelegate
			// 4. 调用 UGameplayStatic::ApplyRadialDamageWithFalloff来造成伤害（这将导致TakeDamage被受害者调用，然后广播OnDamageDelegate）
			// 5. 在 lambda 中，设置 DamageTypeValue 为接收到的伤害值

			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetAvatar))
			{
				CombatInterface->GetOnDamageSignature().AddLambda([&](float DamageAmount)
				{
					DamageTypeValue = DamageAmount;
				});
			}
			/*
			 * 在指定的范围内施加一个带有衰减效果的辐射伤害。该函数允许你通过辐射区域的中心以及范围来计算伤害的衰减效果，使得距离伤害源越远的目标承受的伤害越小。
			 * 通常用于类似爆炸、范围伤害、或者其他类型的伤害传播情境。比如当爆炸发生时，伤害通常会在爆炸源附近最大，然后逐渐向外衰减。
			 */
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				TargetAvatar,
				DamageTypeValue,
				0.f,
				UAuraAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle),
				UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle),
				UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle),
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				SourceAvatar,
				nullptr);
		}
		
		// 将每种属性伤害值合并进行后续计算
		Damage += DamageTypeValue;
	}
	// 2. 获取目标身上的格挡几率，并确定是否成功格挡，如果格挡成功，伤害会减少
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	Damage = bBlocked ? Damage / 2.f : Damage;
	// 3. 获取目标身上的护甲
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);
	// 4. 获取自身的护甲穿透（忽略目标身上的一定比例的护甲）
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluateParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);
	// 5. 获取比例因子
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
	const FRealCurve* CriticalResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalResistanceCoefficient = CriticalResistanceCurve->Eval(TargetPlayerLevel);
	// 6. (100 - SourceArmorPenetration * 0.25f) / 100.f：这部分计算了护甲穿透后的百分比。
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	// 7. 计算伤害减免的百分比。0.333f 是一个调整系数，用来在游戏中平衡伤害与护甲之间的关系。
	Damage *= ((100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f);
	// 8. 获取自身的暴击概率
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluateParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(0.f, SourceCriticalHitChance);
	// 9. 获取目标的暴击抗性
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluateParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(0.f, TargetCriticalHitResistance);
	// 10. 获取自身的暴击伤害
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluateParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.f, SourceCriticalHitDamage);
	// 11. 计算暴击概率（暴击抗性降低一定百分比的暴击几率）和是否暴击
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
	// 12. 计算暴击伤害：双倍伤害加上暴击伤害加成
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	// 将计算结果写入输出
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	
	/*
	// 在执行计算中进行实际捕获
	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, Armor);
	++Armor;
	Armor = FMath::Max<float>(0.f, Armor);
	// 创建一个游戏修改器的评估数据对象，用于描述要应用的修改器的效果。
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, Armor);
	// 将 EvaluatedData 添加到输出执行结果中
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	*/ 
}

void UExecCalc_Damage::DetermineDeBuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluateParameters) const
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	// 根据伤害类型是否赋值来判断是否要应用负面效果
	for(TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDeBuff)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DeBuffType = Pair.Value;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1);
		//如果负面效果设置了伤害，即使为0，也需要应用负面效果
		if(TypeDamage > 0.5f)
		{
			// 获取释放着身上的debff命中率
			const float SourceDeBuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.DeBuff_Chance, false, -1);
			// 获取受击者身上的伤害抗性值
			float TargetDeBuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistance[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TagsToCaptureDefs[ResistanceTag], EvaluateParameters, TargetDeBuffResistance);
			TargetDeBuffResistance = FMath::Max<float>(TargetDeBuffResistance, 0.f);
			// 计算debuff应用概率
			const float EffectiveDeBuffChance = SourceDeBuffChance * (100 - TargetDeBuffResistance) / 100.f;
			// 计算是否命中
			if(const bool bDeBuff = FMath::RandRange(1, 100) < EffectiveDeBuffChance)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				
				const float DeBuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.DeBuff_Damage, false, -1);
				const float DeBuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.DeBuff_Duration, false, -1);
				const float DeBuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.DeBuff_Frequency, false, -1);

				UAuraAbilitySystemLibrary::SetIsSuccessfulDeBuff(ContextHandle, bDeBuff);
				UAuraAbilitySystemLibrary::SetDeBuffDamage(ContextHandle, DeBuffDamage);
				UAuraAbilitySystemLibrary::SetDeBuffDuration(ContextHandle, DeBuffDuration);
				UAuraAbilitySystemLibrary::SetDeBuffFrequency(ContextHandle, DeBuffFrequency);
				UAuraAbilitySystemLibrary::SetDeBuffDamageType(ContextHandle, DamageType);
			}
		}
	}
}
	