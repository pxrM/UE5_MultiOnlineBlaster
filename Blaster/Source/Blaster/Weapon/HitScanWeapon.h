// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 射线类武器
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


private:
	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;


};
