// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRotationSway/WeaponRotationSwayFragment_Interp.h"

#include "Kismet/KismetMathLibrary.h"

void UWeaponRotationSwayFragment_Interp::WeaponRotationSwayTick_Implementation(float DeltaSeconds,FRotator CurrtRot, bool bHasInput,USkeletalMeshComponent* WeaponMesh,bool bInvertRollPitch,bool bDebug)
{
	if (DeltaSeconds <= 0.f) return;
	
	CalculateControllerRotParams(DeltaSeconds,CurrtRot,bUseInputLowpass);

	FVector swayOffsetTarget;
	FRotator swayRotTarget;
	CalculateRotSwayTarget(swayOffsetTarget,swayRotTarget,bInvertRollPitch);
	
	if (SwayOffset!=swayOffsetTarget)
	{
		SwayOffset=UKismetMathLibrary::VInterpTo(SwayOffset,swayOffsetTarget,DeltaSeconds,bHasInput?InterpSpeed_Pos_Input:InterpSpeed_Pos_NoInput);
	}
	if (SwayRot!=swayRotTarget)
	{
		SwayRot=UKismetMathLibrary::RInterpTo(SwayRot,swayRotTarget,DeltaSeconds,bHasInput?InterpSpeed_Rot_Input:InterpSpeed_Rot_NoInput);
	}
	CalPivotRotAndOffset(WeaponMesh,bInvertRollPitch, bDebug);
	
	
}
