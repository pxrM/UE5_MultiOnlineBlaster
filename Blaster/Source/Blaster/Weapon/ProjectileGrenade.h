// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 手榴弹，反弹一会再爆炸
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;


protected:
	virtual void BeginPlay() override;

	/**
	 * 当抛射物与场景发生碰撞并发生“弹跳”（bounce）时，该事件会被自动触发。
	 * @param ImpactResult 碰撞/射线检测结果的核心结构体。
	 * @param ImpactVelocity 表示抛射物在碰撞瞬间的速度向量（单位：厘米/秒）。(这是碰撞前的瞬时速度，不是反弹后的速度。作用：速度越大，弹跳越高/音效越响。)
	 */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);


private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound; //反弹音效

};
