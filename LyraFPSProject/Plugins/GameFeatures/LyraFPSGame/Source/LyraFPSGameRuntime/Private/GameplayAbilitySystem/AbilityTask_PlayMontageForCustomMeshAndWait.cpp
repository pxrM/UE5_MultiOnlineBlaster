


#include "GameplayAbilitySystem/AbilityTask_PlayMontageForCustomMeshAndWait.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "GameplayAbilitySystem/LyraFPSAbilitySystemComponent.h"

UAbilityTask_PlayMontageForCustomMeshAndWait::UAbilityTask_PlayMontageForCustomMeshAndWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Mesh(nullptr)
	, MontageToPlay(nullptr)
	, Rate(1.f)
	, StartSectionName(NAME_None)
	, bStopWhenAbilityEnds(true)
	, bReplicateMontage(true)
	, AnimRootMotionTranslationScale(1.f)
	, bMontageInterrupted(false)
	, bIsPlaying(false)
{
}

UAbilityTask_PlayMontageForCustomMeshAndWait* UAbilityTask_PlayMontageForCustomMeshAndWait::PlayMontageForMeshAndWait(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	USkeletalMeshComponent* InMesh,
	UAnimMontage* InMontageToPlay,
	float InRate,
	FName InStartSectionName,
	bool InbStopWhenAbilityEnds,
	float InAnimRootMotionTranslationScale,
	bool InbReplicateMontage)
{
	UAbilityTask_PlayMontageForCustomMeshAndWait* MyTask = NewAbilityTask<UAbilityTask_PlayMontageForCustomMeshAndWait>(OwningAbility, TaskInstanceName);
	MyTask->Mesh = InMesh;
	MyTask->MontageToPlay = InMontageToPlay;
	MyTask->Rate = InRate;
	MyTask->StartSectionName = InStartSectionName;
	MyTask->bStopWhenAbilityEnds = InbStopWhenAbilityEnds;
	MyTask->AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
	MyTask->bReplicateMontage = InbReplicateMontage;
	return MyTask;
}

UAnimInstance* UAbilityTask_PlayMontageForCustomMeshAndWait::GetTargetAnimInstance() const
{
	if (!Mesh)
	{
		return nullptr;
	}
	return Mesh->GetAnimInstance();
}

UAbilitySystemComponent* UAbilityTask_PlayMontageForCustomMeshAndWait::GetAbilitySystemComponentChecked() const
{
	check(Ability);
	UAbilitySystemComponent* ASC = Ability->GetCurrentActorInfo()->AbilitySystemComponent.Get();
	check(ASC);
	return ASC;
}

void UAbilityTask_PlayMontageForCustomMeshAndWait::Activate()
{
	if (!Ability || !Mesh || !MontageToPlay)
	{
		EndTask();
		return;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		EndTask();
		return;
	}

	ULyraFPSAbilitySystemComponent* ASC = Cast<ULyraFPSAbilitySystemComponent>(AbilitySystemComponent.Get());
	if (!ASC)
	{
		EndTask();
		return;
	}

	// 播放蒙太奇并注册回调
	float Duration = ASC->PlayMontageForMesh(Ability, Mesh, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSectionName, bReplicateMontage);
	if (Duration > 0.f)
	{
		bPlayedMontage = true;
		AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageBlendingOut);
		AnimInstance->OnMontageEnded.AddDynamic(this, &UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageEnded);
	}
	else
	{
		EndTask();
	}
}




void UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != MontageToPlay)
	{
		return;
	}

	// If interrupted, broadcast interrupted; else broadcast blendout
	if (bInterrupted)
	{
		OnInterrupted.Broadcast(true);
	}
	else
	{
		OnBlendOut.Broadcast(false);
	}

	// Mark interrupted if applicable
	if (bInterrupted)
	{
		bMontageInterrupted = true;
	}
}

void UAbilityTask_PlayMontageForCustomMeshAndWait::OnAbilityMontageEnded(UGameplayAbility* EndedAbility, const FGameplayAbilityActivationInfo& ActivationInfo)
{
	if (EndedAbility == Ability)
	{
		OnCompleted.Broadcast();
		EndTask();
	}
}


void UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != MontageToPlay)
		return;

	if (bInterrupted)
		OnInterrupted.Broadcast(true);
	else
		OnCompleted.Broadcast();

	EndTask();

	// ✅ 主动同步本地回调到其他客户端（如果是 Authority）
	if (Ability && Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		// 可通过 ASC RPC 向远端广播一个 "MontageEnd" 事件
		// 或使用 GameplayCue 通知
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(),FGameplayTag::RequestGameplayTag(FName("Event.MontageEnded")),FGameplayEventData());
	}
}

void UAbilityTask_PlayMontageForCustomMeshAndWait::OnPredictiveMontageRejected()
{
	// Prediction was rejected -> stop local montage preview
	if (bIsPlaying)
	{
		UAnimInstance* AnimInstance = GetTargetAnimInstance();
		if (AnimInstance && MontageToPlay)
		{
			AnimInstance->Montage_Stop(0.2f, MontageToPlay);
		}
	}

	// Broadcast cancelled
	OnCancelled.Broadcast();

	// End the task
	EndTask();
}

void UAbilityTask_PlayMontageForCustomMeshAndWait::OnDestroy(bool bInOwnerFinished)
{
	// Remove delegate bindings and optionally stop montage
	UAnimInstance* AnimInstance = GetTargetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageBlendingOut);
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UAbilityTask_PlayMontageForCustomMeshAndWait::OnMontageEnded);
		
		// Stop montage if requested and still playing, and if owner finished (or task destroyed)
		if (bStopWhenAbilityEnds && MontageToPlay && AnimInstance->Montage_IsPlaying(MontageToPlay))
		{
			// Use small blend out time
			AnimInstance->Montage_Stop(0.25f, MontageToPlay);
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}
