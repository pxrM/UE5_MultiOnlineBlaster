// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"


class UNiagaraSystem;


// 标签对应蒙太奇动画的接口体，为了在普攻GA里兼容多种攻击方式
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	// 使用的蒙太奇 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage;

	// 对应tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	// 识别socket的tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	// 攻击时触发伤害的骨骼插槽名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CombatTipSocketName;

	// 攻击音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 获取等级
	virtual int32 GetPlayerLevel();
	
	// 获取武器上的插槽位置 (BlueprintNativeEvent，会通过蓝图初始化虚函数，可以在蓝图中覆写（如果在蓝图中覆写，C++版本的实现将会失效）)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& SocketTag);

	// 更新角色面向攻击目标的方向
	// BlueprintImplementableEvent：具体实现由蓝图来决定，而不是在C++中直接实现
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);

	// 受击蒙太奇
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	// server：角色死亡
	virtual void Die() = 0;

	// 获取角色是否死亡
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;
	
	// 获取角色的Avatar
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();

	// 获取角色身上设置的多个普攻动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages() const;

	// 获取角色流血效果
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetBloodEffect();

	// 根据动画标签获取 FTaggedMontage
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	// 获取角色的小兵数量
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();
	// 增加小兵数量
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementalMinionCount(const int32 Amount);
};
