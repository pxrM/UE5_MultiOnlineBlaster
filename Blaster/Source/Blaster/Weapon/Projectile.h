// Fill out your copyright notice in the Description page of Project Settings.
/*
	射弹物体
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override; //会在所有机器上调用

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void CollideManifestation(); //碰撞后的表现

	void SpawnTrailSystem(); //生成拖尾特效

	void StartDestroyTimer(); //启动延迟销毁计时器
	void TrailDestroyTimerFinished(); //拖尾特效延迟销毁计时器完成回调

	void ExplodeDamage();


protected:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		float DamageVal = 20.f; //伤害

	//拖尾特效管理
	//UNiagaraSystem 是用于创建和管理 Niagara 系统的类。Niagara 是Ue的一个强大的特效系统，用于创建高度可定制的粒子和特效。
	//UNiagaraSystem 包含了一系列粒子模块和参数，用于定义特效的行为、外观和交互。
	//UNiagaraSystem 可以被用于创建和管理一个或多个 UNiagaraComponent 实例，并在场景中进行实例化和播放。
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//是一个 Unreal Engine 中的组件类，用于在场景中放置和控制 Niagara 系统。
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //用于实现子弹、火箭等射弹物体运动的组件类

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMash;

	UPROPERTY(EditAnywhere)
		float DamageInnerRadius = 200.f; //范围伤害内半径
	UPROPERTY(EditAnywhere)
		float DamageOuterRadius = 500.f; //范围伤害外半径

	UPROPERTY(EditAnywhere)
		float InitialSpeed = 15000.f; //弹丸的移动速度

	/* 与服务器端倒带一起使用 */
	/// <summary>
	/// 是否启用服务器倒带验证
	/// </summary>
	bool bUseServerSideRewind = false;
	/// <summary>
	/// 开始发射位置（FVector_NetQuantize整数）
	/// </summary>
	FVector_NetQuantize TraceStart;
	/// <summary>
	/// 子弹初始发射速度（FVector_NetQuantize100精确到小数点后两位）
	/// </summary>
	FVector_NetQuantize100 InitialVelocity;


private:
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //用于创建和管理粒子特效的类，子弹飞行时用
	UPROPERTY()
		class UParticleSystemComponent* TracerComponent; //粒子系统相关的组件类。该类可以被用于将 "UParticleSystem" 创建的粒子特效附加到游戏中的角色、场景、物体等上

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles; //撞击时产生的特效

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;  //撞击时产生的音效

	
	FTimerHandle TrailDestroyTimer; //拖尾特效延迟销毁计时器
	UPROPERTY(EditAnywhere)
		float TrailDestroyTime = 3.f;//延迟销毁时间
};
