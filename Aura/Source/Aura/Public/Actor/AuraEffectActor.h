// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"


class UGameplayEffect;
/*
 * GameplayEffect简称GE，它是技能Buff、被动技能、技能伤害等各种游戏效果的抽象。
 * 通用actor，应用gas某种GE对属性做出改变
 * 
 * UGameplayEffect类中的 DurationPolicy 属性用于控制游戏效果的持续时间策略。
 * 这个属性确定了游戏效果是瞬时的（instant）、永久的（infinite）、或者有限持续时间的。
 * DurationPolicy 属性有三种可能的取值：
 *		Instant: 表示游戏效果是瞬时的，一旦应用即立即生效，然后立刻结束。
 *		Infinite: 表示游戏效果是永久的，会一直持续下去，直到被移除或者取消。
 *		Timed Duration（有限持续时间）: 表示游戏效果有一个预定的持续时间，在这段时间内效果会持续生效，之后效果会自动失效结束。
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();
	

protected:
	virtual void BeginPlay() override;

	// 向目标角色应用一个游戏效果（Gameplay Effect）
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	
protected:
	// 立即生效的Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	// 有限持续时间的Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
};
