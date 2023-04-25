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
	//���ڶ���һ��ģ����TSubclassOf��ʵ������ģ�����ģ�����ΪAProjectile����ʾ��ģ����ľ���������AProjectile�������ࡣ
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectile> ProjectileClass;

};
