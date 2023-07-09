// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GamePlayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMash->SetupAttachment(RootComponent);
	ProjectileMash->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)  //客户端
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComp = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,//要播放的音频资源
			GetRootComponent(),//获取当前Actor的根组件，将声音附加到根组件上
			FName(),//附加点名称
			GetActorLocation(),//获取当前Actor的位置，用于确定声音的位置
			EAttachLocation::KeepWorldPosition,//指定附加位置为世界坐标
			false,//附加物体被销毁时是否自动停止播放
			1.f,//体积倍增
			1.f,//音量倍增
			0.f,//从0位置开始播放
			LoopingSoundAttenuation,//声音的衰减设置
			(USoundConcurrency*)nullptr,//并发声音
			false //表示不自动激活声音组件
		);
	}
}

void AProjectileRocket::Destroyed()
{
	Super::Destroyed();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit self"));
		return;
	}

	ExplodeDamage();

	StartDestroyTimer();

	//播放碰撞视觉效果
	CollideManifestation();

	//隐藏火箭的mesh
	if (ProjectileMash)
	{
		ProjectileMash->SetVisibility(false);
	}

	//停止产生粒子
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	//碰撞后停止播放循环声音，因为这里延迟了子弹销毁，所以声音在延迟时间会一直循环播放，需要自己关闭下
	if (ProjectileLoopComp && ProjectileLoopComp->IsPlaying())
	{
		ProjectileLoopComp->Stop();
	}
}