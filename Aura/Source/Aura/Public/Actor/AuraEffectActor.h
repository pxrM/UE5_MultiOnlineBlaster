// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"


class UAbilitySystemComponent;
class UGameplayEffect;

// 添加ge时机
UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply,
};

// 移除ge时机
UENUM(BlueprintType)
enum class EEffectRemovalPolicy
{
	RemoveOnEndOverlap,
	DoNotRemove,
};

/*
 * 通用 actor，应用gas某种GE对属性做出改变（GameplayEffect简称GE，它是技能Buff、被动技能、技能伤害等各种游戏效果的抽象。）
 *
 * GameplayEffects通过Modifiers 和Executions (GameplayEffectExecutionCalculation) 改变Attributes。
 * 
 * UGameplayEffect类中的 DurationPolicy 属性用于控制游戏效果的持续时间策略。
 * 这个属性确定了游戏效果是立即的（instant）、永久的（infinite）、或者有限持续时间的。
 * DurationPolicy 属性有三种可能的取值：
 *		Instant: 表示游戏效果是瞬时的，一旦应用即立即生效，然后立刻结束。
 *		Infinite: 表示游戏效果是永久的，会一直持续下去，直到被移除或者取消。
 *		Duration（有限持续时间）: 表示游戏效果有一个预定的持续时间，在这段时间内效果会持续生效，之后效果会自动失效结束。
 *	 Periodic：
 *		Duration和Infinite 有 Periodic（周期效果）配置项，可以通过配置 Period 每隔x秒周期性的执行 Modifiers 和 Executions。
 *		周期性效果可以看作是 Instant Effects，每次修改属性的 BaseValue 并且执行 GameplayCues。这对实现持续伤害效果非常有用。
 *
 * Stacking：叠加规则配置，多个技能、Buff、或多重伤害的叠加
 *		StackingType:
 *			AggregateBySource：在自身堆栈叠加，根据Source ASC拥有的GE数量判断
 *			AggregateByTarget：在目标堆栈叠加，根据Target ASC拥有的GE数量判断。
 *		StackLimitCount：可叠加的最大层数，如果连续Apply该Effect则只有小于StackLimitCount的Effect的Stack是有效的
 *		StackDurationRefreshPolicy 每次ApplyEffect时是否刷新Effect的持续时间，即使层数不增加也会刷新Duration
 *		StackPeriodResetPolicy：每次ApplyEffect时是否刷新周期时间（Period），即使层数不增加也会刷新Period
 *		StackExpirationPolicy：当Effect的Duration结束时，层数的结束方式
 *			ClearEntireStack：当有效的游戏效果过期时，整个堆栈将被清除
 *			RemoveSingleStackAndRefreshDuration：结束时减少一层（当前堆栈计数将减少1），然后重新经历一个Duration（持续时间刷新），一直持续到层数减为0
 *			RefreshDuration：直接刷新时间，相当于无限的效果，需要通过回调（Callback-OnStackCountchange)来手动减少堆栈
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();
	

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	// 向目标角色应用一个游戏效果（Gameplay Effect）
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

	//调用此函数，Actor开始自动旋转
	UFUNCTION(BlueprintCallable)
	void StartRotation();
	
	//调用此函数，Actor开始自动更新上下位置
	UFUNCTION(BlueprintCallable)
	void StartSinusoidalMovement();


private:
	// 每一帧更新Actor的位置和转向
	void ItemMovement(float DeltaSeconds);
	
	
protected:
	// 立即生效的Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;

	// 立即生效的Effect的添加时机
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	
	// 有限持续时间的Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;

	// 有限持续时间的Effect的添加时机
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	
	// 永久的Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

	// 永久Effect的添加时机
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	// 永久Effect的移除时机
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

	
	// 激活的ge句柄map
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	// GE的效果等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Applied Effects")
	float ActorLevel = 1.f;

	// Instant和Duration的GE在应用后，此物体是否需要被销毁
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectApplication = false;

	// 敌人是否能够拾取此物体
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bApplyEffectsToEnemy = false;


	// 是否启用旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	bool bRotates = false;
	
	// 旋转速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float RotationRate = 45.f;

	// 是否启用正弦运动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	bool bSinusoidalMovement = false;

	// 正弦幅度，-1到1，调整更新移动范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float SineAmplitude = 1.f;

	// 正弦周期，会影响上下摆动的速度。默认值为1秒一个循环（2PI走完一个正弦的循环，乘以时间，就是一秒一个循环）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float SinePeriod = 1.f;

	// 计算后的Actor所在的位置
	UPROPERTY(BlueprintReadOnly)
	FVector CalculatedLocation;

	// 计算后的Actor的旋转
	UPROPERTY(BlueprintReadOnly)
	FRotator CalculatedRotation;


private:
	//当前掉落物的存在时间，可以通过此时间实现动态效果
	float RunningTime = 0.f;

	// Actor生成的默认初始位置
	FVector InitialLocation;
};
