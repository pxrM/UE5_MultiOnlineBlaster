// P

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UAbilityInfoData;
struct FWidgetControllerParams;
struct FGameplayEffectContextHandle;
class AAuraHUD;
class USpellMenuWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
class UOverlayWidgetController;

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 * @param: AAuraHUD*& 地址引用，可以修改它的地址
	 * @return: bool 判断参数是否获取成功
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD);
	/*
	 * 获取一个UOverlayWidgetController
	 * @param:WorldContextObject 因为静态函数本身不属于ue的任何一个world，所以需要指定一个world上下文进行追踪该world中obj
	 * BlueprintPure 是一个标记，用于标识一个函数或方法是“纯粹”的，即该函数不会修改对象的状态，也不会对外部状态产生影响，它只依赖于输入参数来计算输出结果。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	// 获取一个UAttributeMenuWidgetController
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	// 为角色初始化配置的默认属性
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, const ECharacterClassType CharacterClass, const float Level, UAbilitySystemComponent* ASC);

	// 应用actor的默认能力
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, const ECharacterClassType CharacterClass);

	// 获取角色类型信息表
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	// 获取角色技能配置
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfoData* GetAbilityInfo(const UObject* WorldContextObject);

	// 获取上下文是否阻挡成功
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	
	// 获取上下文是否暴击
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取当前GE是否成功应用负面效果
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDeBuff(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取当前GE负面效果伤害
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取当前GE负面效果持续时间
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取当前GE负面效果触发间隔
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取当前GE负面效果伤害类型
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDeBuffDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	// 获取死亡冲击方向
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	// 设置上下文是否阻挡成功。UPARAM(ref) 表示使用引用传入的参数，而不是一个out参数，在蓝图里会出现在左侧。
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsBlockedHit);

	// 设置上下文是否暴击。UPARAM(ref) 表示使用引用传入的参数，而不是一个out参数，在蓝图里会出现在左侧。
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,
	                             const bool bInIsCriticalHit);
	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDeBuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsSuccessfulDeBuff);

	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InDeBuffDamage);
	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InDeBuffDuration);
	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InDeBuffFrequency);
	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuffDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDeBuffDamageType);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InDeathImpulse);

	//获取攻击位置指定半径内的所有动态Actor
	// WorldContextObject: 世界上下文
	// OutOverlappingActors: 查找结果列表
	// ActorsToIgnore: 需要忽略的actors
	// Radius: 查找半径
	// SphereLocation: 查找位置
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayerWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors,
	                                      const TArray<AActor*>& ActorsToIgnore, float Radius,
	                                      const FVector& SphereOrigin);

	// 双方是否是敌对
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(const AActor* FirstActor, const AActor* SecondActor);

	// 获取经验奖励配置
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClassType CharacterClass, int32 CharacterLevel);
	
	// 应用伤害效果
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);
};
