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

	//��ɫװ�������ɷ��������ƣ�����ֻͬ�����Լ������ǵ���Ҳ��Ҫ������������������������ͬ�������пͻ���
	DOREPLIFETIME(UCombatComponent, EquippedWeapon); 
	//������������Լ����Ʊ������ĵģ�ֻ�з�������Ҹ����˱�����֪ͨ�������ͻ��ˣ�
	//���������ͻ��˱������ĺ���Ϊ���ڱ��أ����Բ��ᴥ����������Ҫ�ӿͻ��˻�ȡ��Ϣͬ���������������������ı����Ż���
	//ʹ��RPC�������ABlasterCharacter::ServerEquipBtnPressed_Implementation()һ��
	DOREPLIFETIME(UCombatComponent, bAiming); 
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//��ȡ��Ϊ RightHand_Socket ���ֳֹ������
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

void UCombatComponent::SetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}
