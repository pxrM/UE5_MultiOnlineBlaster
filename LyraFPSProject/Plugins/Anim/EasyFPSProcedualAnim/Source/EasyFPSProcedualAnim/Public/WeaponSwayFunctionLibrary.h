// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponSwayFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UWeaponSwayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "WeaponSway")
	static FRotator RotatorSpringInterp(const FRotator& Current, const FRotator& Target, FRotatorSpringState& State,
							 float Stiffness, float Damping, float DeltaTime, float Mass,FRotator MaxRot,float SwayRotMaxDeltaSeconds=.05f);
	
	/**
	 * 绕任意点 PivotPoint旋转
	 * @param Pivot 要绕的点
	 * @param DeltaRot 旋转角度
	 * @param OutPosition 位置
	 * @param OutRotation 旋转
	 * @param InPosition 中心点坐标，默认0
	 * @param InRotation 中心点旋转，默认0
	 */
	UFUNCTION(BlueprintCallable, Category = "WeaponSway",meta=(AdvancedDisplay="InPosition,InRotation"))
	static void RotateAroundPivot(FVector Pivot,const FRotator& DeltaRot,FVector& OutPosition,FRotator& OutRotation,const FVector InPosition=FVector::ZeroVector,const FRotator InRotation=FRotator::ZeroRotator);
	
	/**
	 * 模拟手部自然微抖，让武器不死板。
	 * @param NoiseTime		记录Noise的变量
	 * @param DeltaTime		间隔时间
	 * @param NoiseAmount	抖动幅度
	 * @param NoiseSpeed	抖动速率
	 * @return 旋转偏移
	 */
	UFUNCTION(BlueprintCallable, Category = "WeaponSway")
	static FRotator MicroSwayNoise(float &NoiseTime,float DeltaTime,UPARAM(meta=(ClampMin=0.f,ClampMax=1.f))float NoiseAmount=.1f,float NoiseSpeed=2.f);
	
};
