// Fill out your copyright notice in the Description page of Project Settings.
/*
	���񵯣�����һ���ٱ�ը
*/

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity); //�����ص�


private:
	UPROPERTY(EditAnywhere)
		USoundCue* BounceSound; //������Ч

};
