// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Gameframework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	// �洢Ԥ��·���Ĳ���
	FPredictProjectilePathParams PathParams;
	// ����ʹ����ײͨ����Ԥ��·��
	PathParams.bTraceWithChannel = true;
	// ����ʹ����ײ�����Ԥ��·��
	PathParams.bTraceWithCollision = true;
	// ��������Ļ�ϻ���·���ĳ���ʱ�䣨����Ϊ��λ��
	PathParams.DrawDebugTime = 5.f;
	// ��������Ļ�ϻ���·���ķ�ʽ����һ��ʱ���ڻ��ƣ�
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	// �����׳�������ٶ�ʸ��������ʹ���˵�ǰ��ɫ��ǰ������������һ������
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	// ����Ԥ����ʱ�䣨����Ϊ��λ��
	PathParams.MaxSimTime = 4.f;
	// �����׳�����İ뾶���⽫Ӱ��·����Ԥ����
	PathParams.ProjectileRadius = 5.f;
	// ����Ԥ���Ƶ��
	PathParams.SimFrequency = 30.f;
	// �����׳��������ʼλ�ã�����ʹ���˵�ǰ��ɫ��λ��
	PathParams.StartLocation = GetActorLocation();
	// ��������Ԥ��·������ײͨ��������ʹ����Visibilityͨ��
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	// ����ǰ��ɫ��ӵ������б��У��Ա�����Ԥ��·��ʱ��������ײ
	PathParams.ActorsToIgnore.Add(this);

	// �洢Ԥ��·���Ľ��
	FPredictProjectilePathResult PathResult;

	// ��Ԥ��·�����������ͽ���ṹ�崫��
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); //Owner�� AProjectileWeapon::Fire =�� UCombatComponent::EquipWeapon��ָ��Ϊ��ɫ
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//����ֻ��֪ͨ���ã���Ҫ�˺����պ���
			//��OtherActor������˺���DamageVal���˺�ֵ��OwnerController������˺��Ŀ�������this��������˺��Ķ���UDamageType::StaticClass()�������ʹ�õ��˺�����
			UGameplayStatics::ApplyDamage(OtherActor, DamageVal, OwnerController, this, UDamageType::StaticClass());
		}
	}

	// ��Ϊsuper����destroy��������Ҫ�ŵ�����ִ��
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
