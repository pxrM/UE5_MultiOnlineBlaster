// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	//������ײ����Ϊ ECC_WorldDynamic����ʾ����ײ���� WorldDynamic ���͵����������ײ��
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//������ײ�ɷ�QueryAndPhysics ��ʾ����ײ����ܷ���������ײ���ֿ��Ա����������ѯ�Ƿ�����ײ��
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//���ø���ײ�������� Channel ����ײ��Ӧ��ʽΪ ECR_Ignore������������ Channel ����ײ��
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//�ֱ���� ECC_Visibility �� ECC_WorldStatic ���� Channel ����������ײ��Ӧ��ʽΪ ECR_Block��
	//��ʾ����ײ��� Visibility��WorldStatic ���������͵����巢����ײ�󣬽����赲��
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

