// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRotationSway/WeaponRotationSwayFragment_Force.h"

#include "Kismet/KismetMathLibrary.h"

UWeaponRotationSwayFragment_Force::UWeaponRotationSwayFragment_Force()
{
	// SwayRotMulti = FRotator(.0008f, .0003f, .0012);
	bUsePos=false;
	bUseRot=false;
}


void UWeaponRotationSwayFragment_Force::WeaponRotationSwayTick_Implementation(float DeltaSeconds, FRotator CurRot ,bool bHasInput ,USkeletalMeshComponent* WeaponMesh , bool bInvertRollPitch, bool bDebug)
{
	CalculateControllerRotParams(DeltaSeconds, CurRot, bUseInputLowpass);
	// ---- Inertia (mouse → angular velocity) ----
	WeaponAngVel.Roll += -ControllerRotSpeed_Rate.X * InputForce.Roll;
	WeaponAngVel.Pitch += -ControllerRotSpeed_Rate.Y * InputForce.Pitch;
	WeaponAngVel.Yaw += -ControllerRotSpeed_Rate.Z * InputForce.Yaw;


	// ---- Return force ----
	FRotator ReturnForce=ReturnForce_HasInput;
	if (!bHasInput)
	{
		ReturnForce=ReturnForce_NoInput;
	}
	WeaponAngVel.Roll += -WeaponRot.Roll * DeltaSeconds * ReturnForce.Roll;
	WeaponAngVel.Pitch += -WeaponRot.Pitch * DeltaSeconds * ReturnForce.Pitch;
	WeaponAngVel.Yaw += -WeaponRot.Yaw * DeltaSeconds * ReturnForce.Yaw;

	
	

	// ---- Damping ----
	WeaponAngVel.Roll *= FMath::Exp(-Damp.Roll * DeltaSeconds);
	WeaponAngVel.Pitch *= FMath::Exp(-Damp.Pitch * DeltaSeconds);
	WeaponAngVel.Yaw *= FMath::Exp(-Damp.Yaw * DeltaSeconds);

	// ---- Integrate ----
	WeaponRot.Roll += WeaponAngVel.Roll * DeltaSeconds;
	WeaponRot.Pitch += WeaponAngVel.Pitch * DeltaSeconds;
	WeaponRot.Yaw += WeaponAngVel.Yaw * DeltaSeconds;

	
	// ---- 强制慢慢回正到Straighten_MinValue ----
	if (bHasInput&&Straighten_TotalTime!=0)
	{
		WeaponRot *= FMath::Max(FMath::Pow((Straighten_TotalTime - InputTime) / Straighten_TotalTime, Straighten_NoneLinear), Straighten_MinValue);
	}
	
	// ---- Safety clamp ----
	if (WeaponRotClamp!=FRotator::ZeroRotator)
	{
		WeaponRot.Roll  = WeaponRotClamp.Roll==0? WeaponRot.Roll: FMath::Clamp(WeaponRot.Roll,  -WeaponRotClamp.Roll, WeaponRotClamp.Roll);
		WeaponRot.Pitch = WeaponRotClamp.Pitch==0? WeaponRot.Pitch: FMath::Clamp(WeaponRot.Pitch, -WeaponRotClamp.Pitch, WeaponRotClamp.Pitch);
		WeaponRot.Yaw   = WeaponRotClamp.Yaw==0? WeaponRot.Yaw: FMath::Clamp(WeaponRot.Yaw,   -WeaponRotClamp.Yaw, WeaponRotClamp.Yaw);
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

// void UWeaponRotationSwayFragment_Force::MouseInputHelp()
// {
// 	float Speed = ControllerRotSpeed.Size();
// 	float Accel = ControllerRotAcceleration.Size();
// 	float Power = FVector2D::DotProduct(ControllerRotSpeed, ControllerRotAcceleration);
//
// 	switch (InputPhase)
// 	{
// 	case ActiveControl:
// 		if (Power < 0.f)
// 			InputPhase = PassiveDecay;
// 		break;
//
// 	case PassiveDecay:
// 		if (Accel > CorrectiveAccelThreshold)
// 			InputPhase = CorrectiveBurst;
// 		else if (Speed < StopSpeed && Accel < StopAccel && PassiveTime > StopTime)
// 			InputPhase = Stop;
// 		break;
//
// 	case CorrectiveBurst:
// 		InputPhase = ActiveControl;
// 		break;
//
// 	case Stop:
// 		if (Accel > RestartAccelThreshold)
// 			InputPhase = ActiveControl;
// 		break;
// 	}
//
// }
