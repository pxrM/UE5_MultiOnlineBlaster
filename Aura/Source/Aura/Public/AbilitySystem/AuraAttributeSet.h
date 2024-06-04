// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AuraAttributeSet.generated.h"


// 使用该宏为UAuraAttributeSet生成Health的get、set、init等函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(): SourceAsc(nullptr), SourceAvatarActor(nullptr), SourceController(nullptr),
	                     SourceCharacter(nullptr),
	                     TargetAsc(nullptr),
	                     TargetAvatarActor(nullptr),
	                     TargetController(nullptr),
	                     TargetCharacter(nullptr)
	{
	}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceAsc;
	UPROPERTY()
	AActor* SourceAvatarActor;
	UPROPERTY()
	AController* SourceController;
	UPROPERTY()
	ACharacter* SourceCharacter;
	
	UPROPERTY()
	UAbilitySystemComponent* TargetAsc;
	UPROPERTY()
	AActor* TargetAvatarActor;
	UPROPERTY()
	AController* TargetController;
	UPROPERTY()
	ACharacter* TargetCharacter;
};


/**
 * 属性集
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAuraAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 修改属性之前执行。例如：修改血量时进行限制，HP=Clamp(0,MaxHP);
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 修改属性后执行，注意：在效果应用时不会执行，在效果执行时才会执行。
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;


private:
	// 设置ge的相关属性
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;


public:
	UFUNCTION()
	void OnRep_OnStrength(const FGameplayAttributeData& OldStrength) const;
	UFUNCTION()
	void OnRep_OnResilience(const FGameplayAttributeData& OldResilience) const;
	UFUNCTION()
	void OnRep_OnIntelligence(const FGameplayAttributeData& OldIntelligence) const;
	UFUNCTION()
	void OnRep_OnVigor(const FGameplayAttributeData& OldVigor) const;

	UFUNCTION()
	void OnRep_OnArmor(const FGameplayAttributeData& OldArmor) const;
	UFUNCTION()
	void OnRep_OnArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;
	UFUNCTION()
	void OnRep_OnBlockChance(const FGameplayAttributeData& OldBlockChance) const;
	UFUNCTION()
	void OnRep_OnCriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;
	UFUNCTION()
	void OnRep_OnCriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;
	UFUNCTION()
	void OnRep_OnCriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;
	UFUNCTION()
	void OnRep_OnHealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;
	UFUNCTION()
	void OnRep_OnManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;
	UFUNCTION()
	void OnRep_OnMaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_OnMaxMana(const FGameplayAttributeData& OldMaxMana) const;
	
	UFUNCTION()
	void OnRep_OnHealth(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_OnMana(const FGameplayAttributeData& OldMana) const;
	

public:
	/*
	 * Primary
	 */
	
	// 力量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnStrength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
	// 韧性
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnResilience, Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);
	// 智力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnIntelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);
	// 精力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnVigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);


	/*
	 * Secondary
	 */
	
	// 护甲，依赖 Resilience 属性，作用：减少伤害提高格挡几率
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnArmor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);
	// 护甲穿透，依赖 Resilience，忽略一定比例的护甲并增加暴击
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnArmorPenetration, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);
	// 格挡几率，依赖 Armor 属性，有几率减少一半的伤害
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnBlockChance, Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);
	// 暴击几率，依赖 Armor Penetration 属性，伤害有概率双倍暴击
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnCriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);
	// 暴击伤害，依赖 Armor Penetration 属性，暴击时增加额外伤害
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnCriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);
	// 暴击抵抗，依赖 Armor 属性，降低敌人的暴击几率
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnCriticalHitResistance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);
	// 生命值恢复，依赖 Vigor 属性，每秒恢复的生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnHealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);
	// 法力恢复，依赖 Intelligence 属性，每秒恢复的法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);
	// 最大健康，依赖 Vigor 属性，可获得的最大生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMaxHealth, Category = "Secondary Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
	// 最大法力，依赖 Intelligence 属性，可获得的最大法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMaxMana, Category="Secondary Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);
	

	/*
	 * Vital
	 */
	
	// 当前健康属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnHealth, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	// 当前法力属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMana, Category="Mana Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

};
