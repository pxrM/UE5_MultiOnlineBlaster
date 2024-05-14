// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"


// 使用该宏为UAuraAttributeSet生成Health的get、set、init等函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAuraAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_OnHealth(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_OnMaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_OnMana(const FGameplayAttributeData& OldMana) const;
	UFUNCTION()
	void OnRep_OnMaxMana(const FGameplayAttributeData& OldMaxMana) const;
	

public:
	// 当前健康属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnHealth, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	// 最大健康属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
	
	// 当前法力属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMana, Category="Mana Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);
	// 最大法力属性值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OnMaxMana, Category="Mana Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);
};
