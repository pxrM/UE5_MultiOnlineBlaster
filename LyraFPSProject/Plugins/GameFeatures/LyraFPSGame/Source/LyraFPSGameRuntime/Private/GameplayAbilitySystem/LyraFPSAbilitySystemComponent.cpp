// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/LyraFPSAbilitySystemComponent.h"

#include "LyraFPSFunctionLibrary.h"
#include "Animation/LyraAnimInstance.h"
#include "Gameplay/Cosmetic/SkeletalMeshComponent_FPS.h"


// Sets default values for this component's properties
ULyraFPSAbilitySystemComponent::ULyraFPSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void ULyraFPSAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	if (InAvatarActor)
	{
		if (auto curFPSMesh= ULyraFPSFunctionLibrary::GetFPSMesh(InAvatarActor))
		{
			if (auto LyraAnimInst= Cast<ULyraAnimInstance>(curFPSMesh->GetAnimInstance()))
			{
				LyraAnimInst->InitializeWithAbilitySystem(this);
			}
		}
	}
}


float ULyraFPSAbilitySystemComponent::PlayMontageForMesh(
	UGameplayAbility* InAbility,
	USkeletalMeshComponent* InMesh,
	FGameplayAbilityActivationInfo ActivationInfo,
	UAnimMontage* Montage,
	float PlayRate,
	FName StartSectionName,
	bool bReplicateMontage)
{
	if (!InMesh || !Montage) return -1.f;

	UAnimInstance* AnimInstance = InMesh->GetAnimInstance();
	if (!AnimInstance) return -1.f;

	float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
	if (Duration <= 0.f) return -1.f;

	// 记录 Montage 信息
	FGameplayAbilityLocalAnimMontage LocalInfo;
	LocalInfo.AnimMontage = Montage;
	LocalInfo.AnimatingAbility = InAbility;
	MeshMontageMap.Add(InMesh, LocalInfo);

	// 网络同步
	if (IsOwnerActorAuthoritative() && bReplicateMontage)
	{
		// TODO: 实现针对 Mesh 的 Rep 数据包，可参考 Lyra 的 FGameplayAbilityRepAnimMontageForMesh
	}

	// 跳段
	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
	}

	return Duration;
}

