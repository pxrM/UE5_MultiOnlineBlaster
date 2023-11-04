// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"


AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
}

void AFlag::OnEquippedState()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
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

void AFlag::Dropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::ResetFlag()
{
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	if (FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	if (!HasAuthority()) return;

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EWS_Initial);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	SetActorTransform(InitialTransform);
}
