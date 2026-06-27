// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "CurveFloat_ProceduralAnim.generated.h"

/**
 * Curve起点=0 终点=1
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UCurveFloat_ProceduralAnim : public UCurveFloat
{
	GENERATED_BODY()
public:
	//获得当前速度
	UFUNCTION(BlueprintPure)
	float GetVelocity(float Time,float DeltaTime=.01f) const;
};
