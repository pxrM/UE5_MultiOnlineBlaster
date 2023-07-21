// Fill out your copyright notice in the Description page of Project Settings.
/*
ECR_Ignore: ��ʾ��ײ��ӦӦΪ��������ͨ����
ECR_Overlap: ��ʾ��ײ��ӦӦΪ�����ص��¼���������������ĳ��ʱ����ڲ��ֻ�ȫ���ص�����
ECR_Block: ��ʾ��ײ��ӦӦΪ��ֹ����ͨ����
ECR_MAX: ��ʾ��ö�����͵����ֵ��
*/

#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponent/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//bReplicates���ڿ����Ƿ��������縴�ơ�
	//Ϊtrue�����Actor��״̬�����Խ��ڿͻ��˺ͷ�����֮����и��ƺ�ͬ����ȷ���������������֮�������ͬ��״̬����Ϊ��
	//Ϊfalse����Actorֻ�����ڷ������ϣ��ͻ����޷����ʻ��������
	bReplicates = true;
	//ͬ���ƶ�
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);	//�� WeaponMesh ����Ϊ�����������ζ��WeaponMesh�����Ǹ��������Ҫ�������������λ��ȡ���ڸ������λ�á�
	/*������������Ʒ�����֮��Ľ���*/
	//���� WeaponMesh ��������ײͨ�������� Block ��Ӧ, ����ֹĳЩ����ͨ��������ȷ���������ᴩ�����
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//���� WeaponMesh �� Pawn �����������ײ��ӦΪ Ignore��WeaponMesh �������� Pawn ��ص��κ���ײ���Ӷ�ʹ��ҿ���ͨ����������ʰȡ���ϵ���Ʒ
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//�� WeaponMesh ����ײ����Ϊ NoCollision��WeaponMesh �����������κ�����ʵ����ײ��
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/*�������������ֵ	======  ��ͼ���ûḲ��c++��������Ҫ�����ͼ����	*/
	//�ø�ģ�͵��Զ������ģ��ֵ��Custom Depth Stencil Value��
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
	//��Ǹ�ģ�͵���Ⱦ״̬��Render State��Ϊ���ࡱ��Dirty�����⽫ǿ����������һ֡������Ⱦ��ģ�ͣ��Ա�Ӧ���µ��Զ������ģ��ֵ��
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	/*AreaSphere ��������ȷ��һ������Χ���Ա�����ܹ��Ӹ÷�Χ��ʰȡ��������Ʒ�����Բ���Ҫ�Ը���Ϸ���������ײ��Ӧ*/
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	//���ú����κ���ײͨ��
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//���ý�ֹ�����κ���ײ

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickpWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())	// GetLocalRole() == ENetRole::ROLE_Authority
	{
		//����������Ƿ������ˣ���ӵ��Ȩ�޵��Ǹ��ͻ���
		//�� AreaSphere ����ײ����״̬����Ϊ QueryAndPhysics���Ա���в�ѯ������ģ�⡣
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//����ײ��Ӧ����Ϊ ECC_Pawn����ʾ Pawn ������ AreaSphere �ص���
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOerlap);
	}

	ShowPickupWidget(false);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, AmmoNum);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOerlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bShow)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShow);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State; //������ֵ�����ı䣬��ͬ�������пͻ��ˣ������ͻ��� OnRep_WeaponState ����

	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		//����˵��� SetCollisionEnabled ����֮�󣬸ú������������пͻ����ϱ����ò�ִ����Ӧ�Ĳ�������ʹ����˺����пͻ��˶�ʹ����ͬ����ײ����
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);  //�ر���ײ���
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority()) //�Ƿ��ڷ�����
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //������������ײ����Ϊ�ɲ�ѯ
		}
		WeaponMesh->SetSimulatePhysics(true); //ģ������Ч��
		WeaponMesh->SetEnableGravity(true); //��������Ч��
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //����Ϊ�ɲ�ѯ��ģ������Ч���Ķ���
		//���� WeaponMesh ��������ײͨ�������� Block ��Ӧ, ����ֹĳЩ����ͨ��������ȷ���������ᴩ�����
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		//���� WeaponMesh �� Pawn �����������ײ��ӦΪ Ignore��WeaponMesh �������� Pawn ��ص��κ���ײ���Ӷ�ʹ��ҿ���ͨ����������ʰȡ���ϵ���Ʒ
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true); //ģ������Ч��
		WeaponMesh->SetEnableGravity(true); //��������Ч��
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //����Ϊ�ɲ�ѯ��ģ������Ч���Ķ���
		//���� WeaponMesh ��������ײͨ�������� Block ��Ӧ, ����ֹĳЩ����ͨ��������ȷ���������ᴩ�����
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		//���� WeaponMesh �� Pawn �����������ײ��ӦΪ Ignore��WeaponMesh �������� Pawn ��ص��κ���ײ���Ӷ�ʹ��ҿ���ͨ����������ʰȡ���ϵ���Ʒ
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::SpeedRound()
{
	/*������ִ��*/
	AmmoNum = FMath::Clamp(AmmoNum - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	AmmoNum = FMath::Clamp(AmmoNum - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_AmmoNum()
{
	/*ͬ�����ͻ���ִ��*/
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(IsAmmoFull() && BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatCmp())
	{
		BlasterOwnerCharacter->GetCombatCmp()->AnimJumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(AmmoNum);
		}
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	SpeedRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	//ʹ�� FDetachmentTransformRules ���͵� DetachRules ���������������ķ������
	//EDetachmentRule::KeepWorld ��ʾ�������ᱣ��������ռ��У����̳����������Actor��λ�á���ת��������Ϣ��
	//�ڶ������� true ��ʾ����ԭʼ������Ϣ��������������ʱ�����������Լ���λ�á���ת��������Ϣ�����ҿ������ɵ�����Ϸ�������ƶ�����ת��
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	//�������ӵ�ǰ�������������Actor�Ϸ��루detach������ʱ�����������һ�������Ķ���
	WeaponMesh->DetachFromComponent(DetachRules);
	//��ӵ�����ÿ�
	SetOwner(nullptr);

	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}
