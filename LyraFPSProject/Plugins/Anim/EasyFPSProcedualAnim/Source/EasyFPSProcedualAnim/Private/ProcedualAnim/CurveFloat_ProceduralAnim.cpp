// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/CurveFloat_ProceduralAnim.h"

float UCurveFloat_ProceduralAnim::GetVelocity(float Time,float DeltaTime) const
{
	float lastTime=Time-DeltaTime;
	if (lastTime>0)
	{
		float curValue = GetFloatValue(Time);
		float lastValue=GetFloatValue(lastTime);
		return (curValue-lastValue)/DeltaTime;
	}
	return 0.f;
}
