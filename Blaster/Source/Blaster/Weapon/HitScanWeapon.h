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
	/**
	 * 武器射击运动轨道
	 * @param TraceStart 开始位置
	 * @param HitTarget 结束位置
	 * @param OutFireHits 命中目标
	 */
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
