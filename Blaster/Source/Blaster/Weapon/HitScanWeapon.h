// Fill out your copyright notice in the Description page of Project Settings.
/*
*	射线武器
*/

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;


protected:
	/// <summary>
	/// 获取一个扩散后的目标方向
	/// </summary>
	/// <param name="TraceStart">起始点</param>
	/// <param name="HitTarget">射线目标点</param>
	/// <returns></returns>
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);


private:
	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;

	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	/* 分赛子弹 */

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseSactter = false;


};
