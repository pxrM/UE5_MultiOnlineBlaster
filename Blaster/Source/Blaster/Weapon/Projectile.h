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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override; //会在所有机器上调用

	virtual void CollideManifestation(); //碰撞后的表现


protected:
	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //用于实现子弹、火箭等射弹物体运动的组件类


private:
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //用于创建和管理粒子特效的类，子弹飞行时用
	UPROPERTY()
		class UParticleSystemComponent* TracerComponent; //粒子系统相关的组件类。该类可以被用于将 "UParticleSystem" 创建的粒子特效附加到游戏中的角色、场景、物体等上

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles; //撞击时产生的特效

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;  //撞击时产生的音效


protected:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		float DamageVal = 20.f; //伤害


protected:
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
