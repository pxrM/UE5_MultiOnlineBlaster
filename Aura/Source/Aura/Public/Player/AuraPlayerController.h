// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"


class AMagicCircleActor;
class UNiagaraSystem;
class UDamageTextComponent;
class USplineComponent;
class UAuraAbilitySystemComponent;
class UAuraInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;


public:
	UAuraAbilitySystemComponent* GetASC();
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(const float DamageAmount, ACharacter* TargetCharacter, const bool bBlockedHit, const bool bCriticalHit);

	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle();
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();
	
	
private: 
	void Move(const FInputActionValue& InputActionValue);
	void CursorTrace();
	void AutoMove();
	void AbilityInputTagPressed(const FGameplayTag InputTag);
	void AbilityInputTagReleased(const FGameplayTag InputTag);
	void AbilityInputTagHeld(const FGameplayTag InputTag);
	void ShiftPressed() { bShiftKeyDown = true; }
	void ShiftReleased() { bShiftKeyDown = false; }
	void UpdateMagicCircleLocation();
	
	
private:
	// 输入映射上下文 用于定义一组输入映射规则，将物理输入设备的输入映射到虚拟输入轴或按键
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;
	
	// 移动输入动作：输入动作用于绑定玩家输入（例如键盘按键、鼠标点击、手柄按钮等）到游戏中的特定行为或事件
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;
	
	// 鼠标点击的射线
	FHitResult CursorHit;
	// 上一个鼠标选中的actor
	IEnemyInterface* LastActor = nullptr;
	// 当前鼠标选中的actor
	IEnemyInterface* CurrActor = nullptr;

	// 玩家的技能输入配置
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	// gas组件
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;


	// 存储目的地
	FVector CachedDestination = FVector::ZeroVector;
	// 点击时间 用来判断是否是短按
	float FollowTime = 0.f;
	// 短按阈值 过多久后不算是短按
	float ShortPressThreshold = 0.5f;
	// 是否在自动移动
	bool bAutoRunning = false;
	// 是否在准备攻击选中的目标
	bool bTargeting = false;
	// 当角色和目标距离在此半径内时，将关闭自动寻路
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	// 动寻路时生成的样条线。用于创建和管理曲线（Spline）。Spline是一种通过插值点之间的平滑曲线来定义路径或形状的方法。
	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> SplineCmp;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	// 按下shift
	bool bShiftKeyDown = false;

	// 显示伤害数值的组件类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircleActor> MagicCircleClass;
	UPROPERTY()
	TObjectPtr<AMagicCircleActor> MagicCircle;
};
