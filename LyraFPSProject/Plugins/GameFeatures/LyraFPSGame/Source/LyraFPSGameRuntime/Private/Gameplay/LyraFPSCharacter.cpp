// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/LyraFPSCharacter.h"

#include "FPSAnimGameplayTags.h"
#include "LyraHeroFPSComponent.h"
#include "Gameplay/LyraFPSCameraComponent.h"
#include "Gameplay/LyraFPSCharacterMovementComponent.h"
#include "ProcedualAnim/FPSProceduralAnimComp.h"


// Sets default values
ALyraFPSCharacter::ALyraFPSCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<ULyraFPSCameraComponent>("CameraComponent")
		.SetDefaultSubobjectClass<ULyraFPSCharacterMovementComponent>(CharacterMovementComponentName))
{

}

ULyraHeroFPSComponent* ALyraFPSCharacter::GetHeroComp() const
{
	return Cast<ULyraHeroFPSComponent>(GetComponentByClass(ULyraHeroFPSComponent::StaticClass())); 
}

void ALyraFPSCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (auto comp= FindComponentByClass<UFPSProceduralAnimComp>())
	{
		comp->SendSignal(FPSAnimGameplayTags::Signal_Crouch);
	}
	
}

void ALyraFPSCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (auto comp= FindComponentByClass<UFPSProceduralAnimComp>())
	{
		comp->SendSignal(FPSAnimGameplayTags::Signal_UnCrouch);
	}
}
