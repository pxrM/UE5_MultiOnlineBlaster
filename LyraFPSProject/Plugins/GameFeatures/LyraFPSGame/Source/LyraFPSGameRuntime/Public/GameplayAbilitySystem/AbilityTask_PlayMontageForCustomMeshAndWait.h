#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Actor.h"
#include "AbilityTask_PlayMontageForCustomMeshAndWait.generated.h"

// Delegate signatures mirror UE's PlayMontageAndWait
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayMontageForMeshDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayMontageForMeshBlendOutDelegate, bool, bInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayMontageForMeshEventDelegate, FName, EventName);

/**
 * AbilityTask: PlayMontageForMeshAndWait
 * 功能：在指定的 USkeletalMeshComponent 上播放 Montage，并等待完成 / 混合出 / 中断 / 取消
 * 行为：尽量与 UAbilityTask_PlayMontageAndWait 保持一致，唯一不同点是可以传入指定的 Mesh（InMesh）
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API UAbilityTask_PlayMontageForCustomMeshAndWait : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_PlayMontageForCustomMeshAndWait(const FObjectInitializer& ObjectInitializer);

	/** 静态工厂函数（Blueprint可用） */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageForCustomMeshAndWait* PlayMontageForMeshAndWait(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		USkeletalMeshComponent* InMesh,
		UAnimMontage* MontageToPlay,
		float Rate = 1.0f,
		FName StartSectionName = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.0f,
		bool bReplicateMontage = true);

	/** Events */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageForMeshDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FPlayMontageForMeshBlendOutDelegate OnBlendOut;

	UPROPERTY(BlueprintAssignable)
	FPlayMontageForMeshBlendOutDelegate OnInterrupted;

	UPROPERTY(BlueprintAssignable)
	FPlayMontageForMeshDelegate OnCancelled;

	// Optional: if you want notify events (mirrors original PlayMontageAndWait's OnNotify)
	UPROPERTY(BlueprintAssignable)
	FPlayMontageForMeshEventDelegate OnNotifyBegin;

protected:
	/** UAbilityTask interface */
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	/** Parameters (存储从工厂函数传入的参数) */
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY()
	float Rate;

	UPROPERTY()
	FName StartSectionName;

	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	bool bReplicateMontage;

	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** Internal state */
	bool bMontageInterrupted;
	bool bIsPlaying;


	/** Prediction key handle (用于客户端预测时挂钩) */
	FPredictionKey MotionPredictionKey;

	/** Internal functions / callbacks */
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnAbilityMontageEnded(UGameplayAbility* EndedAbility, const FGameplayAbilityActivationInfo& ActivationInfo);
	/** Called when a predictive montage is rejected (client prediction failure) */
	void OnPredictiveMontageRejected();

	/** Helpers */
	UAnimInstance* GetTargetAnimInstance() const;
	class UAbilitySystemComponent* GetAbilitySystemComponentChecked() const;


	// New
private:
	bool bPlayedMontage;
};
