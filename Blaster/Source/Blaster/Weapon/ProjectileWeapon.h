// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 射弹武器
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()


public:
	virtual void Fire(const FVector& HitTarget)override;

private:
	//用于定义一个模板类TSubclassOf的实例。该模板类的模板参数为AProjectile，表示该模板类的具体类型是AProjectile或其子类。
	/// <summary>
	/// 客户端和服务器上使用，会进行网络复制
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
	/// <summary>
	/// 服务器倒带使用，不会进行网络复制
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;

};
