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
	/// 武器击中轨道
	/// </summary>
	/// <param name="TraceStart">开始位置</param>
	/// <param name="HitTarget">结束位置</param>
	/// <param name="OutFireHits">命中目标</param>
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutFireHits);


protected:
	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;


private:
	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;


};
