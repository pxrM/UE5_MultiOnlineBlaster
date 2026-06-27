// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/LyraFPSPlayerState.h"

#include "GameplayAbilitySystem/LyraFPSAbilitySystemComponent.h"

ALyraFPSPlayerState::ALyraFPSPlayerState(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<ULyraFPSAbilitySystemComponent>("AbilitySystemComponent"))
{
	
}
