// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AuraAttributeSet.generated.h"

// 使用该宏为UAuraAttributeSet生成属性的property、get、set、init等函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/*存储一个ge的相关属性*/
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


/* 在C++中，typename 关键字有两个主要用途：一是用来声明模板参数，二是用来指示依赖类型（dependent type）。
 *	依赖类型：在模板编程中，经常会遇到依赖名称（dependent names），也就是在模板参数（如类模板参数）的作用域内依赖于模板参数的名称。
 *			编译器在解析这些名称时，需要知道它们是否表示类型。如果编译器不能确定一个名称是否表示类型，则需要使用 typename 来明确指出这一点。
 *	假设我们有一个模板类和一个嵌套类型，如下所示：
 *		template<typename T>
 *		class Outer {
 *			public:
 *			using InnerType = typename T::SomeType;
 *		};
 *		例子中，T::SomeType 是一个依赖名称，因为它依赖于模板参数 T。编译器无法确定 T::SomeType 是否表示类型，除非我们明确指出。因此，我们使用 typename 来告诉编译器 T::SomeType 是一个类型。
 *		如果不使用 typename，编译器会报错。例如，以下代码会导致编译错误：错误：未使用 typename，编译器不知道 T::SomeType 是否为类型
 */
/* 简化对 TBaseStaticDelegateInstance 中 FFuncPtr 类型的引用 */
// typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;


/**
 * 属性集。
 * UAttributeSet是用于管理角色属性的类之一。
 * 它通常用于实现角色的各种属性，例如健康值、魔法值、力量、敏捷度等。UAttributeSet可以帮助开发者管理和追踪角色的属性变化，并且可以与游戏中的其他系统（如伤害计算、效果处理等）进行交互。
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
	void ShowFloatingText(const FEffectProperties& Props, const float Damage, const bool bBlockedHit, const bool bCriticalHit) const;

	
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
	void OnRep_OnFireResistance(const FGameplayAttributeData& OldFireResistance) const;
	UFUNCTION()
	void OnRep_OnLightningResistance(const FGameplayAttributeData& OldLightningResistance) const;
	UFUNCTION()
	void OnRep_OnArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;
	UFUNCTION()
	void OnRep_OnPhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;

	UFUNCTION()
	void OnRep_OnHealth(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_OnMana(const FGameplayAttributeData& OldMana) const;

	
public:
	/*
	 * Primary Attributes
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
	// 精神
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnVigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);


	/*
	 * Secondary Attributes
	 */

	// 护甲，依赖 Resilience 属性，作用：减少伤害提高格挡几率
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnArmor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);
	// 护甲穿透，依赖 Resilience 属性，忽略一定比例的护甲并增加暴击
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
	 * Resistance Attributes
	 */
	
	// 生命值恢复，依赖 Vigor 属性，每秒恢复的生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnFireResistance, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, FireResistance);
	// 法力恢复，依赖 Intelligence 属性，每秒恢复的法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnLightningResistance, Category = "Resistance Attributes")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, LightningResistance);
	// 最大健康，依赖 Vigor 属性，可获得的最大生命值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnArcaneResistance, Category = "Resistance Attributes")
	FGameplayAttributeData ArcaneResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArcaneResistance);
	// 最大法力，依赖 Intelligence 属性，可获得的最大法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnPhysicalResistance, Category="Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, PhysicalResistance);

	
	/*
	 * Vital Attributes
	 */

	// 当前健康属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnHealth, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	// 当前法力属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMana, Category="Mana Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	
	/*
	 * Meta Attributes
	 */
	
	// 伤害元属性：计算传入进来的伤害值（server）
	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);
	// 伤害元属性：传入进来的经验值（server）
	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingXP);
	

public:
	/*
	 * 声明了一个类型为 TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr 的函数指针，名为 FunctionPointer。
	 *	TBaseStaticDelegateInstance: 是UE用于实现委托的一种基础模板类
	 *	FGameplayAttribute(): 这是函数签名，表示返回类型为 FGameplayAttribute 且没有参数的函数
	 *	FDefaultDelegateUserPolicy: 这是一个用户策略类，控制代理实例的行为。
	 *	FFuncPtr: 是 TBaseStaticDelegateInstance 的内部类型，代表函数指针的类型
	 */
	// TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FunctionPointer;
	// C++: TMap<FGameplayTag, FGameplayAttribute(*)()> TestTagsToAttributes;
	// 别名：TMap<FGameplayTag, FAttributeFuncPtr> TagsToAttributes;
	/* 将tag和attribute进行映射 */
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
};
