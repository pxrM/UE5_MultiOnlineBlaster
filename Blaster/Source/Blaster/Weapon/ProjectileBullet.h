// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet();

#if WITH_EDITOR	// 编辑器
	/// <summary>
	/// 在编辑器中修改对象属性后执行特定逻辑的回调函数
	/// </summary>
	/// <param name="Event"></param>
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif // !


protected:
	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)override;
	
};
