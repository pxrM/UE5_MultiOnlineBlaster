// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraBeamSpell.generated.h"


/**
 * 光线射击技能
 */
UCLASS()
class AURA_API UAuraBeamSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * 将鼠标拾取命中信息存储
	 * @param HitResult 在技能中通过TargetDataUnderMouse的task获取到的结果
	 */
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);
	/*
	 * 设置拥有该技能玩家的一些变量
	 */
	UFUNCTION(BlueprintCallable)
	void StoreOwnerVariables();
	/**
	 * 获取闪电命中的第一个目标
	 * @param BeamTargetLocation 鼠标点击目标位置
	 * @note 如果鼠标拾取和武器发射位置中间有其它敌人被阻挡，更新目标位置。
	 */
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget(const FVector& BeamTargetLocation);
	/**
	 * 通过技能命中目标获取扩散的敌人目标
	 * @param OutAdditionalTargets 返回获取到的最近的目标数组
	 */
	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);

protected:
	// 鼠标选中的位置
	UPROPERTY(BlueprintReadWrite, Category="Beam")
	FVector MouseHitLocation;

	// 鼠标选中的actor
	UPROPERTY(BlueprintReadWrite, Category="Beam")
	TObjectPtr<AActor> MouseHitActor;

	// 拥有该技能的玩家控制器
	UPROPERTY(BlueprintReadWrite, Category="Beam")
	TObjectPtr<APlayerController> OwnerPlayerController;

	UPROPERTY(BlueprintReadWrite, Category="Beam")
	TObjectPtr<ACharacter> OwnerCharacter;

	// 最大电击连接数
	UPROPERTY(EditDefaultsOnly, Category="Beam")
	int32 MaxNumShockTarget = 5;
};
