// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraHeroFPSComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
ULyraHeroFPSComponent::ULyraHeroFPSComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	// PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULyraHeroFPSComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	Super::Input_Move(InputActionValue);
	MoveInput = InputActionValue.Get<FVector2D>();
}

void ULyraHeroFPSComponent::Input_Move_Cancel(const FInputActionValue& InputActionValue)
{
	Super::Input_Move_Cancel(InputActionValue);
	MoveInput = FVector2D();
}

void ULyraHeroFPSComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	Super::Input_LookMouse(InputActionValue);
	MouseInput = InputActionValue.Get<FVector2D>();
}

void ULyraHeroFPSComponent::Input_LookMouse_Cancel(const FInputActionValue& InputActionValue)
{
	Super::Input_LookMouse_Cancel(InputActionValue);
	MouseInput =  FVector2D();
}


bool ULyraHeroFPSComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
                                               FGameplayTag DesiredState) const
{
	return Super::CanChangeInitState(Manager, CurrentState, DesiredState);
}

void ULyraHeroFPSComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	Super::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

void ULyraHeroFPSComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	Super::OnActorInitStateChanged(Params);
}

void ULyraHeroFPSComponent::CheckDefaultInitialization()
{
	Super::CheckDefaultInitialization();
}




// void ULyraHeroComponent_FPS::InitializePlayerInput(UInputComponent* PlayerInputComponent)
// {
// 	Super::InitializePlayerInput(PlayerInputComponent);
//
//
// }



