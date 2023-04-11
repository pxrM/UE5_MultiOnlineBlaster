// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//角色装备武器由服务器控制，这里只同步了自己，但是敌人也需要看到你的武器，所以这里加上同步给所有客户端
	DOREPLIFETIME(UCombatComponent, EquippedWeapon); 
	//这里是由玩家自己控制变量更改的，只有服务器玩家更改了变量会通知给其他客户端，
	//但是其他客户端变量更改后因为是在本地，所以不会触发，这里需要从客户端获取信息同步到服务器，服务器更改变量才会解决
	//使用RPC解决，像ABlasterCharacter::ServerEquipBtnPressed_Implementation()一样
	DOREPLIFETIME(UCombatComponent, bAiming); 
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//获取名为 RightHand_Socket 的手持骨骼插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName(TEXT("RightHand_Socket")));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
}
	
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (!Character->HasAuthority())
	{
		ServerSetAiming(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}
