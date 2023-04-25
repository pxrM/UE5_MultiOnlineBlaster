// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Gameframework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

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

	//"UProjectileMovementComponent" ��UE�����ڿ���Ͷ���Projectile�����˶��켣������ࡣ
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//��ʾ�����������Ͷ������ƶ���������תͶ�����ʹ��Ͷ�����ڷ��й����У��ܹ�ʼ�ձ��ֳ����ƶ��ķ��򣬿�����������Ȼ��Ҳ�����ڿ��ơ�
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//����������ʱ��������ǰ������һ���켣��Ч�Ĺ���
	if (Tracer)
	{
		//��ʹ�� "UGameplayStatics::SpawnEmitterAttached" ������ "CollisionBox" ���������� "Tracer" ��Ч��
		//���һ������ "EAttachLocation::KeepWorldPosition" ��ʾ����Ч������������ռ��в��䡣����ζ�ţ�����Ч�������ܵ�Դ������κ�ת����ƽ�Ʋ�����Ӱ�졣
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer, CollisionBox, FName(),
			GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

