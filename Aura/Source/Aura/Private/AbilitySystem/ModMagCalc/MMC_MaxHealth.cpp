// P


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// 设置要捕获的属性
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// 属性捕获的来源，比如来源于施法者、目标等
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	// 是否进行快照
	VigorDef.bSnapshot = false;

	// 添加到要就进行捕获的数组中
	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 从Source和Target中收集tag
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 用于在 Aggregator（聚合器）中评估属性值时传递参数。聚合器在游戏中用于计算和管理属性（Attributes）的变化和影响，比如生命值、攻击力等
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 获取捕获属性的值
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluateParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	// 获取player等级
	float PlayerLevel = 1.f;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}
