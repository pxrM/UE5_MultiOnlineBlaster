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
};
