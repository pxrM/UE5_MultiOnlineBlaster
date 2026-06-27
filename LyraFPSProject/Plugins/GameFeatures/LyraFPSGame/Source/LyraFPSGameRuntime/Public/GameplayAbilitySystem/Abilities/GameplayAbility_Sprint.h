// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GameplayAbility_Sprint.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayEventData;
struct FGameplayTagContainer;

class UAbilityTask_WaitInputRelease;
class UAbilityTask_WaitDelay;

USTRUCT()
struct FFPSSprintMovementCache
{
	GENERATED_BODY()

	UPROPERTY()
	float MaxWalkSpeed = 0.0f;

	UPROPERTY()
	float MaxAcceleration = 0.0f;

	UPROPERTY()
	float BrakingDecelerationWalking = 0.0f;

	UPROPERTY()
	float GroundFriction = 0.0f;

	UPROPERTY()
	float BrakingFrictionFactor = 0.0f;

	UPROPERTY()
	FRotator RotationRate = FRotator::ZeroRotator;
};

/** Sprint ability
 * 分三个部分：
 * 1. 奔跑/停止逻辑：	开始奔跑时采用冲刺数据，存下奔跑前数据，并不严谨，但凑合着用
 * 2. 落地状态：		在空中时松开冲刺键应依旧保持速度，奔跑技能不能停，防止空中减速出戏
 * 3. 奔跑按键监听：	在空中时松开冲刺键不能减速，但可以开枪，存下按键状态，开枪不是技能阻挡关系，而是Tag关系。
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API UGameplayAbility_Sprint : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	//开始/结束奔跑
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CharacterSprintStart();
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CharacterSprintStop();
	
	//按键Input监听
	UFUNCTION()
	void OnSprintInputReleased(float TimeHeld);
	void HandleInputReleased();
	void BeginWaitingForGroundedAfterRelease();
	void ClearGroundedReleaseWait();
	void BeginWaitingForInputRelease();
	void ClearInputReleaseWait();
	void SetSprintInputHeld(bool bNewHeld);
	void SetSprintFireBlocked(bool bNewBlocked);
	void UpdateSprintFireBlockState();
	void BeginWatchingSprintFireBlockMovement();
	void ClearSprintFireBlockMovementWatch();
	void BeginSprintJumpFireDelayWait();
	void ClearSprintJumpFireDelayWait();
	UFUNCTION()
	void OnSprintJumpFireDelayElapsed();
	void TryEndSprintAfterInputReleased();
	
	//落地Tag监听
	void OnGroundedMovementTagChanged(const FGameplayTag Tag, int32 NewCount);
	void OnSprintFireBlockMovementTagChanged(const FGameplayTag Tag, int32 NewCount);
	bool HasGroundedMovementTag() const;
	bool HasFallingMovementTag() const;
	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsOnGrounded() const;
	bool IsOnGrounded(const FGameplayAbilityActorInfo* ActorInfo) const;
	
protected:

	bool bInputReleased = false;
	bool bSprintInputHeld = false;
	bool bSprintFireBlocked = false;
	bool bWaitingForGroundedAfterRelease = false;
	bool bWatchingSprintFireBlockMovement = false;
	bool bSprintJumpFireDelayElapsed = false;
	FDelegateHandle GroundedWalkTagHandle;
	FDelegateHandle GroundedNavWalkTagHandle;
	FDelegateHandle SprintFireBlockFallingTagHandle;
	FDelegateHandle SprintFireBlockWalkTagHandle;
	FDelegateHandle SprintFireBlockNavWalkTagHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> WaitSprintJumpFireDelayTask;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint|Jump")
	bool bAllowFireAfterSprintJumpDelay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint|Jump", meta=(EditCondition="bAllowFireAfterSprintJumpDelay", ClampMin="0.0"))
	float SprintJumpFireDelay = 0.2f;
	
};
