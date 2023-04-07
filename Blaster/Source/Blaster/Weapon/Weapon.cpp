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

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//bReplicates���ڿ����Ƿ��������縴�ơ�
	//Ϊtrue�����Actor��״̬�����Խ��ڿͻ��˺ͷ�����֮����и��ƺ�ͬ����ȷ���������������֮�������ͬ��״̬����Ϊ��
	//Ϊfalse����Actorֻ�����ڷ������ϣ��ͻ����޷����ʻ��������
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);	//�� WeaponMesh ����Ϊ�����������ζ��WeaponMesh�����Ǹ��������Ҫ�������������λ��ȡ���ڸ������λ�á�

	/*������������Ʒ�����֮��Ľ���*/
	//���� WeaponMesh ��������ײͨ�������� Block ��Ӧ, ����ֹĳЩ����ͨ��������ȷ���������ᴩ�����
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//���� WeaponMesh �� Pawn �����������ײ��ӦΪ Ignore��WeaponMesh �������� Pawn ��ص��κ���ײ���Ӷ�ʹ��ҿ���ͨ����������ʰȡ���ϵ���Ʒ
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//�� WeaponMesh ����ײ����Ϊ NoCollision��WeaponMesh �����������κ�����ʵ����ײ��
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
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
		//�ر���ײ���
		//����˵��� SetCollisionEnabled ����֮�󣬸ú������������пͻ����ϱ����ò�ִ����Ӧ�Ĳ�������ʹ����˺����пͻ��˶�ʹ����ͬ����ײ����
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}



