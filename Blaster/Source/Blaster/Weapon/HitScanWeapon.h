// Fill out your copyright notice in the Description page of Project Settings.
/*
*	��������
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
	/// ��ȡһ����ɢ���Ŀ�귽��
	/// </summary>
	/// <param name="TraceStart">��ʼ��</param>
	/// <param name="HitTarget">����Ŀ���</param>
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

	/* �����ӵ� */

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseSactter = false;


};
