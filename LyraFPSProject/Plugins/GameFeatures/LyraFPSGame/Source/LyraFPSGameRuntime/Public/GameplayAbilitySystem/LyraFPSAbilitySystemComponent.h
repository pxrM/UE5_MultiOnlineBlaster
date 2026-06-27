// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "LyraFPSAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraFPSAbilitySystemComponent : public ULyraAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraFPSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
public:
	// 播放指定 Mesh 的蒙太奇
	float PlayMontageForMesh(
		UGameplayAbility* InAbility,
		USkeletalMeshComponent* InMesh,
		FGameplayAbilityActivationInfo ActivationInfo,
		UAnimMontage* Montage,
		float PlayRate,
		FName StartSectionName,
		bool bReplicateMontage);

protected:
	// 用于存储多个 Mesh 的播放状态
	UPROPERTY()
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FGameplayAbilityLocalAnimMontage> MeshMontageMap;
};
