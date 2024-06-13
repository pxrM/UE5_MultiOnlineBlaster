// P


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	// 设置角色的移动是否与其朝向相关联。true，角色将会朝着它的移动方向旋转，这意味着当角色移动时，它会自动朝向移动的方向。
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置角色的旋转速率。它定义了角色在每个轴上旋转的速度。这里角色在 Yaw 轴上的旋转速度被设置为 400 度/秒，而在其他轴上的旋转速度被设置为零，意味着在 Pitch 和 Roll 轴上角色不会自动旋转。
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	// 角色是否限制在一个平面上移动。设置为true，则角色的移动将被限制在一个平面上，而不是在整个三维空间中自由移动。
	GetCharacterMovement()->bConstrainToPlane = true;
	// 设置角色在开始时是否立即贴合到限制的平面上。设置为true，则角色在开始时将立即贴合到限制的平面上。
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	/*
	 * 分别设置了是否允许使用控制器的旋转来控制角色的 Pitch、Roll 和 Yaw 轴。
	 */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 为server初始化AbilityInfo
	InitAbilityActorInfo();
	// server初始化角色能力
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 为client初始化AbilityInfo
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// 在多人游戏中这个可能返回为null，只有本地玩家有
	if(AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if(AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	InitializeDefaultAttributes();
}
