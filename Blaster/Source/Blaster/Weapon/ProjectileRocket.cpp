// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GamePlayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Roket Mesh"));
	RocketMash->SetupAttachment(RootComponent);
	RocketMash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)  //�ͻ���
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	if (TrailSystem)
	{
		//�ڳ����и��Ӳ�����һ�� Niagara ϵͳ
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,//Ҫ���ɵ� Niagara ϵͳ��ָ�롣
			GetRootComponent(),//����ȷ��Ҫ�������ɵ� Niagara ϵͳ��λ�õĸ������
			FName(),//���ӵ����ƣ�����ָ�����������߲����
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,//������������ռ��е�λ�á�
			false //��ʾ���ɵ� Niagara ϵͳ�������Զ����ٹ��ܡ�
		);
	}

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComp = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,//Ҫ���ŵ���Ƶ��Դ
			GetRootComponent(),//��ȡ��ǰActor�ĸ���������������ӵ��������
			FName(),//���ӵ�����
			GetActorLocation(),//��ȡ��ǰActor��λ�ã�����ȷ��������λ��
			EAttachLocation::KeepWorldPosition,//ָ������λ��Ϊ��������
			false,//�������屻����ʱ�Ƿ��Զ�ֹͣ����
			1.f,//�������
			1.f,//��������
			0.f,//��0λ�ÿ�ʼ����
			LoopingSoundAttenuation,//������˥������
			(USoundConcurrency*)nullptr,//��������
			false //��ʾ���Զ������������
		);
	}
}

void AProjectileRocket::Destroyed()
{
	Super::Destroyed();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//��ȡ������ҵĿ�����
	APawn* FiringPawn = GetInstigator(); //SpawnParams.Instigator = InstigatorPawn;
	if (FiringPawn && HasAuthority()) //������ִ�еĴ����
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

	//��Ӽ�ʱ���ӳ�������ʧ
	GetWorldTimerManager().SetTimer(
		TrailDestroyTimer,
		this,
		&AProjectileRocket::TrailDestroyTimerFinished,
		TrailDestroyTime
	);

	//������ײ�Ӿ�Ч��
	CollideManifestation();

	//���ػ����mesh
	if (RocketMash)
	{
		RocketMash->SetVisibility(false);
	}

	//ֹͣ��������
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	//��ײ��ֹͣ����ѭ����������Ϊ�����ӳ����ӵ����٣������������ӳ�ʱ���һֱѭ�����ţ���Ҫ�Լ��ر���
	if (ProjectileLoopComp && ProjectileLoopComp->IsPlaying())
	{
		ProjectileLoopComp->Stop();
	}
}

void AProjectileRocket::TrailDestroyTimerFinished()
{

}
