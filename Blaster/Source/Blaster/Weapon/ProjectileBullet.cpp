// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Gameframework/ProjectileMovementComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()); //Owner在 AProjectileWeapon::Fire =》 UCombatComponent::EquipWeapon中指定为角色
	if (OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			// 在服务器上且没有启用倒带
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind) 
			{
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamageVal : DamageVal;
				//这里只是通知作用，需要伤害接收函数
				//对OtherActor造成了伤害，DamageVal是伤害值，OwnerController是造成伤害的控制器，this代表造成伤害的对象，UDamageType::StaticClass()则代表所使用的伤害类型
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				// 因为super里有destroy，所以需要放到后面执行
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComp() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComp()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
	}
	// 因为super里有destroy，所以需要放到后面执行
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}


void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
	// 存储预测路径的参数
	FPredictProjectilePathParams PathParams;
	// 开启使用碰撞通道来预测路径
	PathParams.bTraceWithChannel = true;
	// 开启使用碰撞检测来预测路径
	PathParams.bTraceWithCollision = true;
	// 设置在屏幕上绘制路径的持续时间（以秒为单位）
	PathParams.DrawDebugTime = 5.f;
	// 设置在屏幕上绘制路径的方式（在一定时间内绘制）
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	// 设置抛出物体的速度矢量，这里使用了当前角色的前向向量并乘以一个常数
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	// 设置预测的最长时间（以秒为单位）
	PathParams.MaxSimTime = 4.f;
	// 设置抛出物体的半径，这将影响路径的预测结果
	PathParams.ProjectileRadius = 5.f;
	// 设置预测的频率
	PathParams.SimFrequency = 30.f;
	// 设置抛出物体的起始位置，这里使用了当前角色的位置
	PathParams.StartLocation = GetActorLocation();
	// 设置用于预测路径的碰撞通道，这里使用了Visibility通道
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	// 将当前角色添加到忽略列表中，以避免在预测路径时与自身碰撞
	PathParams.ActorsToIgnore.Add(this);

	// 存储预测路径的结果
	FPredictProjectilePathResult PathResult;

	// 来预测路径，将参数和结果结构体传入
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}
