// Fill out your copyright notice in the Description page of Project Settings.

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


public:	


private:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //用于实现子弹、火箭等射弹物体运动的组件类

	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //用于创建和管理粒子特效的类

	class UParticleSystemComponent* TracerComponent; //粒子系统相关的组件类。该类可以被用于将 "UParticleSystem" 创建的粒子特效附加到游戏中的角色、场景、物体等上

};
