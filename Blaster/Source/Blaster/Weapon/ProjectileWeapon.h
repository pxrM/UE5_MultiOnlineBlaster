// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()


public:
	virtual void Fire(const FVector& HitTarget)override;

private:
	//用于定义一个模板类TSubclassOf的实例。该模板类的模板参数为AProjectile，表示该模板类的具体类型是AProjectile或其子类。
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectile> ProjectileClass;

};
