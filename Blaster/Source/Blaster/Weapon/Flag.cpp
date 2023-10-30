// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::OnEquippedState()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}

void AFlag::OnDroppedState()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //�ڷ�����������������ײ����Ϊ�ɲ�ѯ
	}
	FlagMesh->SetSimulatePhysics(true); //ģ������Ч��
	FlagMesh->SetEnableGravity(true); //��������Ч��
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //����Ϊ�ɲ�ѯ��ģ������Ч���Ķ���
	//���� WeaponMesh ��������ײͨ�������� Block ��Ӧ, ����ֹĳЩ����ͨ��������ȷ���������ᴩ�����
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//���� WeaponMesh �� Pawn �����������ײ��ӦΪ Ignore��WeaponMesh �������� Pawn ��ص��κ���ײ���Ӷ�ʹ��ҿ���ͨ����������ʰȡ���ϵ���Ʒ
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}
