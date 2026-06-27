// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRotationSway/WeaponRotationSwayFragment_SpringInterp.h"

#include "WeaponSwayFunctionlibrary.h"

UWeaponRotationSwayFragment_SpringInterp::UWeaponRotationSwayFragment_SpringInterp()
{
	NonLinearMappingPower=FVector(1.15f);
}

void UWeaponRotationSwayFragment_SpringInterp::WeaponRotationSwayTick_Implementation(float DeltaSeconds, FRotator CurrtRot,
                                                                                     bool bHasInput, USkeletalMeshComponent* WeaponMesh,bool bInvertRollPitch,bool bDebug)
{
	if (DeltaSeconds <= 0.f) return;
	
	// 1) rot speed (deg/s) and smoothing
	CalculateControllerRotParams(DeltaSeconds,CurrtRot, bUseInputLowpass);
	
	// 2) compute  target
	FVector swayOffsetTarget;
	FRotator swayRotTarget;
	CalculateRotSwayTarget(swayOffsetTarget,swayRotTarget,bInvertRollPitch);
	
	// 4) position spring interp
	SwayOffset = UKismetMathLibrary::VectorSpringInterp(SwayOffset, swayOffsetTarget, PosSpringState,
														Pos_Stiffness, Pos_Damping, DeltaSeconds,
														Pos_Mass,TargetVelocityAmount);
	// 4.1) clamp
	if (SwayOffset.Size() > MaxOffsetLength)
	{
		SwayOffset = SwayOffset.GetSafeNormal() * MaxOffsetLength;
	}

	// 5) rotation spring interp
	SwayRot =  UWeaponSwayFunctionLibrary::RotatorSpringInterp(SwayRot, swayRotTarget, RotSpringState, Rot_Stiffness, Rot_Damping,
								  DeltaSeconds, Rot_Mass,MaxRot,SwayRotMaxDeltaSeconds);
	
	// 6) post process
	CalPivotRotAndOffset(WeaponMesh, bInvertRollPitch, bDebug);
}
