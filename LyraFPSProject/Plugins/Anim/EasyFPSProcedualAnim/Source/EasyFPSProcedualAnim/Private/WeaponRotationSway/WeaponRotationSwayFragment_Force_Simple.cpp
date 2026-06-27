// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRotationSway/WeaponRotationSwayFragment_Force_Simple.h"

#include "Kismet/KismetMathLibrary.h"

UWeaponRotationSwayFragment_Force_Simple::UWeaponRotationSwayFragment_Force_Simple()
{
	// SwayRotMulti = FRotator(.0008f, .0003f, .0012);
	bUsePos=false;
	bUseRot=false;
}

void UWeaponRotationSwayFragment_Force_Simple::WeaponRotationSwayTick_Implementation(float DeltaSeconds, FRotator CurRot ,bool bHasInput ,USkeletalMeshComponent* WeaponMesh , bool bInvertRollPitch, bool bDebug)
{
	if (bHasInput)
	{
		InputTime += DeltaSeconds;
	}
	else
	{
		InputTime = 0.f;
	}

	CalculateControllerRotParams(DeltaSeconds, CurRot, bUseInputLowpass);

	// ---- Inertia (mouse → angular velocity) ----
	// CRot/Ds
	curInputForce= -ControllerRotSpeed.X * InputForce.Roll;
	returnForce= -(WeaponRot.Roll / DeltaSeconds * (bHasInput ? ReturnForce_HasInput.Roll : ReturnForce_NoInput.Roll));
	curForce=  curInputForce+returnForce;
	WeaponAngVel.Roll += -ControllerRotSpeed.X * InputForce.Roll;
	WeaponAngVel.Pitch += -ControllerRotSpeed.Y * InputForce.Pitch;
	WeaponAngVel.Yaw += -ControllerRotSpeed.X * InputForce.Yaw;
	
	// ---- Return force ----
	// CRot/Ds * For1 - WRot/DS * For2	
	// 36/DS * 0.1		36/DS	* 0.1
	WeaponAngVel.Roll += -WeaponRot.Roll / DeltaSeconds * (bHasInput ? ReturnForce_HasInput.Roll : ReturnForce_NoInput.Roll);
	WeaponAngVel.Pitch += -WeaponRot.Pitch / DeltaSeconds * (bHasInput ? ReturnForce_HasInput.Pitch : ReturnForce_NoInput.Pitch);
	WeaponAngVel.Yaw += -WeaponRot.Yaw / DeltaSeconds * (bHasInput ? ReturnForce_HasInput.Yaw : ReturnForce_NoInput.Yaw);


	// ---- Damping ----
	//防止return force 反复摇摆
	//只影响速度变化速度
	// if (!bHasInput)
	// {
		WeaponAngVel.Roll *= FMath::Exp(-Damp.Roll * DeltaSeconds);
        WeaponAngVel.Pitch *= FMath::Exp(-Damp.Pitch * DeltaSeconds);
        WeaponAngVel.Yaw *= FMath::Exp(-Damp.Yaw * DeltaSeconds);
	// }
	

	// ---- Integrate ----
	WeaponRot.Roll += WeaponAngVel.Roll * DeltaSeconds;
	WeaponRot.Pitch += WeaponAngVel.Pitch * DeltaSeconds;
	WeaponRot.Yaw += WeaponAngVel.Yaw * DeltaSeconds;

	
	// ---- 强制回正到0.3----
	if (bHasInput&&Straighten_TotalTime!=0)
	{
		WeaponRot *= FMath::Max(FMath::Pow((Straighten_TotalTime - InputTime) / Straighten_TotalTime, Straighten_NoneLinear), Straighten_MinValue);
	}

	// ---- Safety clamp ----
	if (WeaponRotClamp!=FRotator::ZeroRotator)
	{
		WeaponRot.Roll  = FMath::Clamp(WeaponRot.Roll,  -WeaponRotClamp.Roll, WeaponRotClamp.Roll);
		WeaponRot.Pitch = FMath::Clamp(WeaponRot.Pitch, -WeaponRotClamp.Pitch, WeaponRotClamp.Pitch);
		WeaponRot.Yaw   = FMath::Clamp(WeaponRot.Yaw,   -WeaponRotClamp.Yaw, WeaponRotClamp.Yaw);
	}
	
	
	SwayRot = WeaponRot;
	if (bInvertRollPitch)
	{
		float tmp = SwayRot.Pitch;
		SwayRot.Pitch = SwayRot.Roll;
		SwayRot.Roll = tmp;
	}
	CalNoise(DeltaSeconds,bInvertRollPitch,bHasInput);
	CalPivotRotAndOffset(WeaponMesh, bInvertRollPitch, bDebug);
	
}

