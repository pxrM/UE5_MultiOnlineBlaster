// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GamePlayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Roket Mesh"));
	RocketMash->SetupAttachment(RootComponent);
	RocketMash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//��ȡ������ҵĿ�����
	APawn* FiringPawn = GetInstigator(); //SpawnParams.Instigator = InstigatorPawn;
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// ����˥��Ч���ľ����˺�
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //����������
				DamageVal,	//�����˺�
				10.f, //��С�˺�
				GetActorLocation(),	//�˺���Χ��Բ�ĵ�
				200.f, //�˺��ڰ뾶
				500.f, //�˺���뾶
				1.f, //˥�����ӣ������˺���������Ӷ�˥�����ٶ�
				UDamageType::StaticClass(), //ҪӦ�õ��˺�����
				TArray<AActor*>(), //�����˺���actor
				this, //�˺�����Դ����
				FiringController //�˺�����Դ����Ŀ�����
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
