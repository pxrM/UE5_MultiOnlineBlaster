// Fill out your copyright notice in the Description page of Project Settings.
/*
	火箭，对一定范围内的角色造成伤害，并对该伤害进行衰减
*/

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();


protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	//拖尾特效延迟销毁计时器完成回调
	void TrailDestroyTimerFinished();


private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* RocketMash;
	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMoveCmp;


protected:
	//拖尾特效管理
	//UNiagaraSystem 是用于创建和管理 Niagara 系统的类。Niagara 是Ue的一个强大的特效系统，用于创建高度可定制的粒子和特效。
	//UNiagaraSystem 包含了一系列粒子模块和参数，用于定义特效的行为、外观和交互。
	//UNiagaraSystem 可以被用于创建和管理一个或多个 UNiagaraComponent 实例，并在场景中进行实例化和播放。
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//是一个 Unreal Engine 中的组件类，用于在场景中放置和控制 Niagara 系统。
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	//拖尾特效延迟销毁计时器
	FTimerHandle TrailDestroyTimer;
	//延迟销毁时间
	UPROPERTY(EditAnywhere)
		float TrailDestroyTime = 3.f;

	UPROPERTY(EditAnywhere)
		USoundCue* ProjectileLoop;
	UPROPERTY()
		UAudioComponent* ProjectileLoopComp;
	UPROPERTY(EditAnywhere)
		USoundAttenuation* LoopingSoundAttenuation; //声音衰减

};
