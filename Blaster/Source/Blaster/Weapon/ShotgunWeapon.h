// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;//--废弃
	virtual void FireShotgun(const TArray<FVector_NetQuantize> HitTargets);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

private:
	UPROPERTY(EditAnywhere)
		uint32 NumberOfPellets = 10; //霰弹枪同时出的子弹数量
	
};
