// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class UAbilityInfoData;
class AAuraHUD;
class USpellMenuWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
class UOverlayWidgetController;
struct FWidgetControllerParams;
struct FGameplayEffectContextHandle;

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 获取一个UOverlayWidgetController
	// WorldContextObject 因为静态函数本身不属于ue的任何一个world，所以需要指定一个world上下文进行追踪该world中obj
	// BlueprintPure 是一个标记，用于标识一个函数或方法是“纯粹”的，即该函数不会修改对象的状态，也不会对外部状态产生影响，它只依赖于输入参数来计算输出结果。
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	// 获取属性菜单的WidgetController
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	// 获取技能菜单的WidgetController
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * 通过角色的数据资产类配置初始化角色属性
	 * @param WorldContextObject  一个世界场景的对象，用于获取当前所在的世界
	 * @param CharacterClass 角色类型
	 * @param Level 角色等级
	 * @param ASC 角色的技能系统组件
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, const ECharacterClassType CharacterClass, const float Level, UAbilitySystemComponent* ASC);

	/**
	 * 通过存档数据来初始化角色的属性
	 * @param WorldContextObject  一个世界场景的对象，用于获取当前所在的世界
	 * @param ASC 角色的技能系统组件
	 * @param SaveGame 角色使用的存档指针
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributesFormSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, const ULoadScreenSaveGame* SaveGame);

	// 应用actor的默认能力
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, const ECharacterClassType CharacterClass);

	// 获取角色类型信息表
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	// 获取角色技能配置，此数据会配置到GameMode上
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfoData* GetAbilityInfo(const UObject* WorldContextObject);

	// 获取战利品数据资产，此数据会配置到GameMode上
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf="WorldContextObject"))
	static ULootTiers* GetLootTiers(const UObject* WorldContextObject);

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

	// 获取受击后击退方向
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	// 当前是否是范围伤害
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	// 范围伤害的原点
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);

	// 范围伤害的内半径
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	// 范围伤害的外半径
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);

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

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InKnockbackForce);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsRadialDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool InIsRadialDamage);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOrigin(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InRadialDamageOrigin);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageInnerRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InRadialDamageInnerRadius);

	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOuterRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,
	                                       const float InRadialDamageOuterRadius);

	
	/**
	 * @param: AAuraHUD*& 地址引用，可以修改它的地址
	 * @return: bool 判断参数是否获取成功
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController",
		meta=(DefaultToSelf="WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams,
	                                       AAuraHUD*& OutAuraHUD);

	/**
	 * 修改伤害配置项，将其设置为具有范围伤害的配置项
	 * @param DamageEffectParams 需要修改的配置
	 * @param bIsRadial 设置是否为范围伤害
	 * @param InnerRadius 内半径
	 * @param OuterRadius 外半径
	 * @param Origin 中心点
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static void SetIsRadialDamageEffectParam(UPARAM(ref) FDamageEffectParams& DamageEffectParams, const bool bIsRadial,
	                                         const float InnerRadius, const float OuterRadius, const FVector Origin);

	/**
	 * 修改伤害时的冲击力的方向
	 * @param DamageEffectParams 需要修改的配置
	 * @param KnockbackDirection 攻击时触发击退的方向
	 * @param Magnitude 幅度
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static void SetKnockbackDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams,
	                                  FVector KnockbackDirection, const float Magnitude = 0.f);

	/**
	 * 修改伤害配置的死亡时触发击退的方向
	 * @param DamageEffectParams 需要修改的配置
	 * @param ImpulseDirection 死亡时触发击退的方向
	 * @param Magnitude 幅度
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static void SetDeathImpulseDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams,
	                                     FVector ImpulseDirection, const float Magnitude = 0.f);

	/**
	 * 设置伤害配置的应用目标ASC
	 * @param DamageEffectParams 
	 * @param InAsc 
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static void SetEffectParamTargetAsc(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InAsc);

	
	/**
	 * 获取攻击位置指定半径内的所有动态Actor
	 * 
	 * @param WorldContextObject 世界上下文
	 * @param OutOverlappingActors 查找结果列表
	 * @param ActorsToIgnore 需要忽略的actors
	 * @param Radius 查找半径
	 * @param SphereOrigin 查找位置
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayerWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors,
	                                      const TArray<AActor*>& ActorsToIgnore, float Radius,
	                                      const FVector& SphereOrigin);

	/**
	 * 获取距离目标位置最近的几个目标
	 * @param MaxTargets 获取最大目标的数量
	 * @param Actors 需要计算的目标数组
	 * @param OutClosestTargets 返回获取到的最近的目标
	 * @param Origin 计算的位置
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(const int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	/**
	 * @param FirstActor 
	 * @param SecondActor 
	 * @return 双方是否敌对
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(const AActor* FirstActor, const AActor* SecondActor);

	/**
	 * @param WorldContextObject 
	 * @param CharacterClass 
	 * @param CharacterLevel 
	 * @return 获取经验奖励配置
	 */
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClassType CharacterClass, int32 CharacterLevel);

	/**
	 * 应用伤害效果
	 * @param DamageEffectParams 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	/**
	 * 根据传入的值计算均匀分布的多段角度，
	 *
	 * @param Forward 正前方向
	 * @param Axis 基于旋转的轴
	 * @param Spread 角度范围
	 * @param NumRotators 分段数
	 *
	 * @return TArray<FRotator&> 返回每段角度的旋转角度
	 *
	 * @note 用于在技能生成投掷物的函数逻辑中。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumRotators);

	/**
	 * 根据传入的值计算均匀分布的多段朝向
	 *
	 * @param Forward 正前方向
	 * @param Axis 基于旋转的轴
	 * @param Spread 角度范围
	 * @param NumVectors 分段数
	 *
	 * @return TArray<FVector&> 返回每段角度的中间角度的朝向数组
	 *
	 * @note 用于在技能生成投掷物的函数逻辑中。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlySpacedVectors(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumVectors);
	
};
