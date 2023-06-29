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
	//获取发射玩家的控制器
	APawn* FiringPawn = GetInstigator(); //SpawnParams.Instigator = InstigatorPawn;
	if (FiringPawn)
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

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
