// Fill out your copyright notice in the Description page of Project Settings.
/*
	�������һ����Χ�ڵĽ�ɫ����˺������Ը��˺�����˥��
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
		USoundAttenuation* LoopingSoundAttenuation; //����˥��

	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMovementComponent;

};
