// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/GameplayAbility_Sprint.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/LyraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/LyraFPSGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Gameplay/LyraFPSCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Sprint)

UGameplayAbility_Sprint::UGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	SetAssetTags(FGameplayTagContainer(LyraFPSGameplayTags::Ablity_Sprint));
	ActivationOwnedTags.AddTag(LyraFPSGameplayTags::Status_Sprinting);
}

bool UGameplayAbility_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const ACharacter* LyraCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!LyraCharacter)
	{
		return false;
	}

	const UCharacterMovementComponent* MoveComp = LyraCharacter->GetCharacterMovement();
	if (!MoveComp || LyraCharacter->IsCrouched())
	{
		return false;
	}

	return IsOnGrounded(ActorInfo);
}

void UGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bInputReleased = false;
	bSprintInputHeld = false;
	bSprintFireBlocked = false;
	bSprintJumpFireDelayElapsed = false;
	ClearGroundedReleaseWait();
	ClearSprintFireBlockMovementWatch();
	
	BeginWatchingSprintFireBlockMovement();
	SetSprintInputHeld(true);
	BeginWaitingForInputRelease();
	CharacterSprintStart();
}

void UGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearInputReleaseWait();
	ClearGroundedReleaseWait();
	ClearSprintFireBlockMovementWatch();
	ClearSprintJumpFireDelayWait();
	SetSprintInputHeld(false);
	SetSprintFireBlocked(false);
	CharacterSprintStop();
	bInputReleased = false;
	bSprintJumpFireDelayElapsed = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Sprint::CharacterSprintStart()
{
	if (auto avatarActor= GetAvatarActorFromActorInfo())
	{
		if (auto moveComp= avatarActor->GetComponentByClass<ULyraFPSCharacterMovementComponent>())
		{
			moveComp->StartSprint();
		}
	}
}

void UGameplayAbility_Sprint::CharacterSprintStop()
{
	// RestoreMovementDefaults();
	if (auto avatarActor= GetAvatarActorFromActorInfo())
	{
		if (auto moveComp= avatarActor->GetComponentByClass<ULyraFPSCharacterMovementComponent>())
		{
			moveComp->StopSprint();
		}
	}
	
}


void UGameplayAbility_Sprint::OnSprintInputReleased(float TimeHeld)
{
	WaitInputReleaseTask = nullptr;
	SetSprintInputHeld(false);
	HandleInputReleased();
}

bool UGameplayAbility_Sprint::IsOnGrounded() const
{
	return IsOnGrounded(GetCurrentActorInfo());
}

bool UGameplayAbility_Sprint::IsOnGrounded(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ALyraCharacter* OwnerChar = ActorInfo ? Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UCharacterMovementComponent* MoveComp = OwnerChar ? OwnerChar->GetCharacterMovement() : nullptr;
	return MoveComp && (MoveComp->MovementMode == MOVE_Walking || MoveComp->MovementMode == MOVE_NavWalking);
}





void UGameplayAbility_Sprint::HandleInputReleased()
{
	if (bInputReleased)
	{
		return;
	}

	bInputReleased = true;
	if (HasGroundedMovementTag() || IsOnGrounded())
	{
		TryEndSprintAfterInputReleased();
		return;
	}

	BeginWaitingForGroundedAfterRelease();
}

void UGameplayAbility_Sprint::BeginWaitingForGroundedAfterRelease()
{
	if (bWaitingForGroundedAfterRelease)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	bWaitingForGroundedAfterRelease = true;
	GroundedWalkTagHandle = ASC->RegisterGameplayTagEvent(LyraGameplayTags::Movement_Mode_Walking)
		.AddUObject(this, &ThisClass::OnGroundedMovementTagChanged);
	GroundedNavWalkTagHandle = ASC->RegisterGameplayTagEvent(LyraGameplayTags::Movement_Mode_NavWalking)
		.AddUObject(this, &ThisClass::OnGroundedMovementTagChanged);

	if (HasGroundedMovementTag() || IsOnGrounded())
	{
		TryEndSprintAfterInputReleased();
	}
}

void UGameplayAbility_Sprint::ClearGroundedReleaseWait()
{
	if (!bWaitingForGroundedAfterRelease)
	{
		GroundedWalkTagHandle.Reset();
		GroundedNavWalkTagHandle.Reset();
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (GroundedWalkTagHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(GroundedWalkTagHandle, LyraGameplayTags::Movement_Mode_Walking);
		}

		if (GroundedNavWalkTagHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(GroundedNavWalkTagHandle, LyraGameplayTags::Movement_Mode_NavWalking);
		}
	}

	GroundedWalkTagHandle.Reset();
	GroundedNavWalkTagHandle.Reset();
	bWaitingForGroundedAfterRelease = false;
}

void UGameplayAbility_Sprint::BeginWaitingForInputRelease()
{
	if (WaitInputReleaseTask)
	{
		return;
	}

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnSprintInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}
}

void UGameplayAbility_Sprint::ClearInputReleaseWait()
{
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}
}

void UGameplayAbility_Sprint::SetSprintInputHeld(bool bNewHeld)
{
	if (bSprintInputHeld == bNewHeld)
	{
		UpdateSprintFireBlockState();
		return;
	}

	bSprintInputHeld = bNewHeld;

	if (ULyraAbilitySystemComponent* LyraASC = Cast<ULyraAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		if (bNewHeld)
		{
			LyraASC->AddDynamicTagGameplayEffect(LyraFPSGameplayTags::Status_SprintInputHeld);
		}
		else
		{
			LyraASC->RemoveDynamicTagGameplayEffect(LyraFPSGameplayTags::Status_SprintInputHeld);
		}
	}

	UpdateSprintFireBlockState();
}

void UGameplayAbility_Sprint::SetSprintFireBlocked(bool bNewBlocked)
{
	if (bSprintFireBlocked == bNewBlocked)
	{
		return;
	}

	bSprintFireBlocked = bNewBlocked;

	if (ULyraAbilitySystemComponent* LyraASC = Cast<ULyraAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		if (bNewBlocked)
		{
			LyraASC->AddLooseGameplayTag(LyraFPSGameplayTags::Status_Sprint_LocalAnim);
			LyraASC->AddDynamicTagGameplayEffect(LyraFPSGameplayTags::Status_SprintFireBlocked);
		}
		else
		{
			LyraASC->RemoveLooseGameplayTag(LyraFPSGameplayTags::Status_Sprint_LocalAnim);
			LyraASC->RemoveDynamicTagGameplayEffect(LyraFPSGameplayTags::Status_SprintFireBlocked);
		}
	}
}

void UGameplayAbility_Sprint::UpdateSprintFireBlockState()
{
	if (!bSprintInputHeld)
	{
		ClearSprintJumpFireDelayWait();
		SetSprintFireBlocked(false);
		return;
	}

	if (!bAllowFireAfterSprintJumpDelay)
	{
		ClearSprintJumpFireDelayWait();
		SetSprintFireBlocked(true);
		return;
	}

	if (!HasFallingMovementTag())
	{
		bSprintJumpFireDelayElapsed = false;
		ClearSprintJumpFireDelayWait();
		SetSprintFireBlocked(true);
		return;
	}

	if (bSprintJumpFireDelayElapsed || SprintJumpFireDelay <= 0.0f)
	{
		bSprintJumpFireDelayElapsed = true;
		ClearSprintJumpFireDelayWait();
		SetSprintFireBlocked(false);
		return;
	}

	BeginSprintJumpFireDelayWait();
	SetSprintFireBlocked(true);
}

void UGameplayAbility_Sprint::BeginWatchingSprintFireBlockMovement()
{
	if (!bAllowFireAfterSprintJumpDelay || bWatchingSprintFireBlockMovement)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	bWatchingSprintFireBlockMovement = true;
	SprintFireBlockFallingTagHandle = ASC->RegisterGameplayTagEvent(LyraGameplayTags::Movement_Mode_Falling)
		.AddUObject(this, &ThisClass::OnSprintFireBlockMovementTagChanged);
	SprintFireBlockWalkTagHandle = ASC->RegisterGameplayTagEvent(LyraGameplayTags::Movement_Mode_Walking)
		.AddUObject(this, &ThisClass::OnSprintFireBlockMovementTagChanged);
	SprintFireBlockNavWalkTagHandle = ASC->RegisterGameplayTagEvent(LyraGameplayTags::Movement_Mode_NavWalking)
		.AddUObject(this, &ThisClass::OnSprintFireBlockMovementTagChanged);
}

void UGameplayAbility_Sprint::ClearSprintFireBlockMovementWatch()
{
	if (!bWatchingSprintFireBlockMovement)
	{
		SprintFireBlockFallingTagHandle.Reset();
		SprintFireBlockWalkTagHandle.Reset();
		SprintFireBlockNavWalkTagHandle.Reset();
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (SprintFireBlockFallingTagHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(SprintFireBlockFallingTagHandle, LyraGameplayTags::Movement_Mode_Falling);
		}

		if (SprintFireBlockWalkTagHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(SprintFireBlockWalkTagHandle, LyraGameplayTags::Movement_Mode_Walking);
		}

		if (SprintFireBlockNavWalkTagHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(SprintFireBlockNavWalkTagHandle, LyraGameplayTags::Movement_Mode_NavWalking);
		}
	}

	SprintFireBlockFallingTagHandle.Reset();
	SprintFireBlockWalkTagHandle.Reset();
	SprintFireBlockNavWalkTagHandle.Reset();
	bWatchingSprintFireBlockMovement = false;
}

void UGameplayAbility_Sprint::BeginSprintJumpFireDelayWait()
{
	if (WaitSprintJumpFireDelayTask)
	{
		return;
	}

	WaitSprintJumpFireDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, SprintJumpFireDelay);
	if (WaitSprintJumpFireDelayTask)
	{
		WaitSprintJumpFireDelayTask->OnFinish.AddDynamic(this, &ThisClass::OnSprintJumpFireDelayElapsed);
		WaitSprintJumpFireDelayTask->ReadyForActivation();
	}
}

void UGameplayAbility_Sprint::ClearSprintJumpFireDelayWait()
{
	if (WaitSprintJumpFireDelayTask)
	{
		WaitSprintJumpFireDelayTask->EndTask();
		WaitSprintJumpFireDelayTask = nullptr;
	}
}

void UGameplayAbility_Sprint::OnSprintJumpFireDelayElapsed()
{
	WaitSprintJumpFireDelayTask = nullptr;
	bSprintJumpFireDelayElapsed = true;
	UpdateSprintFireBlockState();
}

void UGameplayAbility_Sprint::TryEndSprintAfterInputReleased()
{
	if (!IsActive() || !bInputReleased || !(HasGroundedMovementTag() || IsOnGrounded()))
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGameplayAbility_Sprint::OnGroundedMovementTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		TryEndSprintAfterInputReleased();
	}
}

void UGameplayAbility_Sprint::OnSprintFireBlockMovementTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0)
	{
		return;
	}

	if (Tag == LyraGameplayTags::Movement_Mode_Falling
		|| Tag == LyraGameplayTags::Movement_Mode_Walking
		|| Tag == LyraGameplayTags::Movement_Mode_NavWalking)
	{
		bSprintJumpFireDelayElapsed = false;
		UpdateSprintFireBlockState();
	}
}

bool UGameplayAbility_Sprint::HasGroundedMovementTag() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC
		&& (ASC->GetGameplayTagCount(LyraGameplayTags::Movement_Mode_Walking) > 0
			|| ASC->GetGameplayTagCount(LyraGameplayTags::Movement_Mode_NavWalking) > 0);
}

bool UGameplayAbility_Sprint::HasFallingMovementTag() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC && ASC->GetGameplayTagCount(LyraGameplayTags::Movement_Mode_Falling) > 0;
}
