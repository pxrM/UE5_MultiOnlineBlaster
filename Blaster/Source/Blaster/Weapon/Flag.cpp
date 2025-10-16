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
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //在服务器将武器球形碰撞体设为可查询
	}
	FlagMesh->SetSimulatePhysics(true); //模拟物理效果
	FlagMesh->SetEnableGravity(true); //启用重力效果
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //设置为可查询和模拟物理效果的对象
	//设置 WeaponMesh 对所有碰撞通道都具有 Block 响应, 会阻止某些物体通过它，以确保武器不会穿过玩家
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//设置 WeaponMesh 对 Pawn 类型物体的碰撞响应为 Ignore，WeaponMesh 将忽略与 Pawn 相关的任何碰撞，从而使玩家可以通过穿过武器拾取地上的物品
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
	if (ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner()))
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
