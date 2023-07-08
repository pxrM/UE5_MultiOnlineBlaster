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


protected:
	UPROPERTY(EditAnywhere)
		USoundCue* ProjectileLoop;
	UPROPERTY()
		UAudioComponent* ProjectileLoopComp;
	UPROPERTY(EditAnywhere)
		USoundAttenuation* LoopingSoundAttenuation; //声音衰减

	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMovementComponent;

};
