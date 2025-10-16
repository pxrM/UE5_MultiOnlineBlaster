// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

/**
 * 霰弹枪
 */
UCLASS()
class BLASTER_API AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;//--废弃
	/**
	 * 霰弹枪射击
	 * @param HitTargets 射击目标
	 */
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	/**
 	 * 计算霰弹枪的散射终点位置
 	 * 该函数通过在一个球体空间内随机生成多个点，来模拟霰弹枪的散射效果
	 *      玩家 ------> 枪口位置 ------> 散射球体 -----> 目标
     *		(Player)    (TraceStart)    (Sphere)     (HitTarget)
	 * @param HitTarget 玩家瞄准的目标点
	 * @param HitTargets 输出参数，存储所有散射子弹的终点位置
	 */
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets) const;

private:
	/**
	 * 霰弹枪同时出的子弹数量
	 */
	UPROPERTY(EditAnywhere)
	uint32 NumberOfPellets = 10;

};
