// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"

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
	/*
	* ���⣺������ײͨ������׼ȷ����Ϊ��ͨ����ɫ���ϵĽ�������ײ�ģ�������Ĵ�С��ʱ�򲢲���ȫ׼ȷ��������ʱ���ͽ�ɫģ��������������ײ
	*			Ӧ���跨���н�ɫmesh�����ǽ�ɫ���ϵĽ�����
	*			����Ϊ��ɫ����ר�Ŵ���һ���Զ������ײͨ������Ŀ����-Engine-Collision�����ã�
	*/
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	/*�ĵ�������ӣ���֧�ֲ�ͬ���͵��ƶ��ؼ�
	//"UProjectileMovementComponent" ��UE�����ڿ���Ͷ���Projectile�����˶��켣������ࡣ
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//��ʾ�����������Ͷ������ƶ���������תͶ�����ʹ��Ͷ�����ڷ��й����У��ܹ�ʼ�ձ��ֳ����ƶ��ķ��򣬿�����������Ȼ��Ҳ�����ڿ��ơ�
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	*/
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

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit); //��OnHit�����󶨵�CollisionBox�������ײ�¼��ϣ�����ײ�¼�����ʱ�Զ�ִ�иú�����
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//if (BlasterCharacter)
	//{
	//	BlasterCharacter->MulticastHit();
	//} //�����Ϊ�ɸ��½�ɫ����ֵʱ����������ֵ�ᱻͬ�������пͷ��ˣ��ŵ�����������һ������㲥����

	Destroy(); //���ڷ����������пͻ��˽��й㲥
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	CollideManifestation();
}

void AProjectile::CollideManifestation()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

