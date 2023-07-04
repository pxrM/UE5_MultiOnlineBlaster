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
	RocketMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMash->SetupAttachment(RootComponent);
	RocketMash->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if (TrailSystem)
	{
		//在场景中附加并生成一个 Niagara 系统
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,//要生成的 Niagara 系统的指针。
			GetRootComponent(),//用于确定要附加生成的 Niagara 系统的位置的根组件。
			FName(),//附加点名称，可以指定骨骼名或者插槽名
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,//保持其在世界空间中的位置。
			false //表示生成的 Niagara 系统不具有自动销毁功能。
		);
	}

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
	//获取发射玩家的控制器
	APawn* FiringPawn = GetInstigator(); //SpawnParams.Instigator = InstigatorPawn;
	if (FiringPawn && HasAuthority()) //服务器执行的代码块
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// 带有衰减效果的径向伤害
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //世界上下文
				DamageVal,	//基础伤害
				10.f, //最小伤害
				GetActorLocation(),	//伤害范围的圆心点
				200.f, //伤害内半径
				500.f, //伤害外半径
				1.f, //衰减因子，控制伤害随距离增加而衰减的速度
				UDamageType::StaticClass(), //要应用的伤害类型
				TArray<AActor*>(), //忽略伤害的actor
				this, //伤害的来源对象
				FiringController //伤害的来源对象的控制器
			);
		}
	}

	//添加计时器延迟烟雾消失
	GetWorldTimerManager().SetTimer(
		TrailDestroyTimer,
		this,
		&AProjectileRocket::TrailDestroyTimerFinished,
		TrailDestroyTime
	);

	//播放碰撞视觉效果
	CollideManifestation();

	//隐藏火箭的mesh
	if (RocketMash)
	{
		RocketMash->SetVisibility(false);
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

void AProjectileRocket::TrailDestroyTimerFinished()
{
	Destroyed();
}
