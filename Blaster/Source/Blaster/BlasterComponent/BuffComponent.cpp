// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmmo, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmmo / HealingTime;
	AmountToHeal += HealAmmo;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime; //每一帧治疗的数量
	Character->SetCurHealth(FMath::Clamp(Character->GetCurHealth() + HealThisFrame, 0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame; //总治疗量减去已经治疗的

	if (AmountToHeal <= 0.f || Character->GetCurHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}


void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ShieldThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetCurShield(FMath::Clamp(Character->GetCurShield() + ShieldThisFrame, 0, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ShieldThisFrame;

	if (ShieldReplenishAmount <= 0.f || Character->GetCurShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}


void UBuffComponent::SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, BuffTime);

	HandleChangeSpeed(BuffBaseSpeed, BuffCrouchSpeed);
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	HandleChangeSpeed(InitialBaseSpeed, InitialCrouchSpeed);
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation (float BaseSpeed, float CrouchSpeed)
{
	HandleChangeSpeed(BaseSpeed, CrouchSpeed);
}

void UBuffComponent::HandleChangeSpeed(float BaseSpeed, float CrouchSpeed) const
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}


void UBuffComponent::SetInitialJumpVelocity(const float JumpVelocity)
{
	InitialJumpVelocity = JumpVelocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJumpVelocity, BuffTime);

	HandleChangeJumpVelocity(BuffJumpVelocity);
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::ResetJumpVelocity()
{
	HandleChangeJumpVelocity(InitialJumpVelocity);
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	HandleChangeJumpVelocity(JumpVelocity);
}

void UBuffComponent::HandleChangeJumpVelocity(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}
