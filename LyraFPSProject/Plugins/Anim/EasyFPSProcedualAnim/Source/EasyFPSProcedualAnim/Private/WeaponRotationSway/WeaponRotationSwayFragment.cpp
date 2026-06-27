// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRotationSway/WeaponRotationSwayFragment.h"
#include "WeaponSwayFunctionlibrary.h"
#include "GeometryCollection/GeometryCollectionParticlesData.h"
#include "Kismet/KismetMathLibrary.h"


void UWeaponRotationSwayFragment::CalculateControllerRotParams(float DeltaSeconds, FRotator CurrtRot, bool bUseLowpass)
{
	// 1) delta rot
	FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrtRot, ControlLastRot);
	ControlLastRot = CurrtRot;

	// 2) rotSpeed
	FVector2D originRotSpeed(deltaRot.Yaw / DeltaSeconds, deltaRot.Pitch / DeltaSeconds);

	ProcessRotSpeed(originRotSpeed, DeltaSeconds, bUseLowpass);

	//CalRotAcce(DeltaSeconds);
}

void UWeaponRotationSwayFragment::ProcessRotSpeed(const FVector2D& originRotSpeed, float DeltaSeconds, bool bUseLowpass)
{
	// 2.1) low-pass smoothing
	if (bUseLowpass)
	{
		float alpha = FMath::Clamp(DeltaSeconds / InputLowpassThreshold, 0.f, 1.f);
		ControllerRotSpeed.X = FMath::Lerp(ControllerRotSpeed.X, originRotSpeed.X, alpha);
		ControllerRotSpeed.Y = FMath::Lerp(ControllerRotSpeed.Y, originRotSpeed.Y, alpha);
	}
	else
	{
		ControllerRotSpeed.X = originRotSpeed.X;
		ControllerRotSpeed.Y = originRotSpeed.Y;
	}

	// 2.2) Normalize
	ControllerRotSpeedNormalized = FVector2D(ControllerRotSpeed.X / FiducialRotSpeed,
	                                         ControllerRotSpeed.Y / FiducialRotSpeed);
	ControllerRotSpeedNormalized.X = FMath::Clamp(ControllerRotSpeedNormalized.X, -1.f, 1.f);
	ControllerRotSpeedNormalized.Y = FMath::Clamp(ControllerRotSpeedNormalized.Y, -1.f, 1.f);

	
	// 2.3) nonlinear mapping (emphasize higher speeds)
	// NonLinearX: X与Y的差值，防止Pitch时因为X的小幅度转动Roll乱晃,其他两个轴也可以尝试忽略小幅度转动
	// X->Roll Y->Pitch Z->Yaw
	float NonLinearX=pow(FMath::Max( abs(ControllerRotSpeedNormalized.X) - abs(ControllerRotSpeedNormalized.Y),0),1) * FMath::Sign(ControllerRotSpeedNormalized.X);
	auto NonLinear = [](float v, float power) { return FMath::Sign(v) * FMath::Pow(FMath::Abs(v), power); };
	ControllerRotSpeed_NonLinear = FVector(
		NonLinear(NonLinearX, NonLinearMappingPower.X),
		NonLinear(ControllerRotSpeedNormalized.Y, NonLinearMappingPower.Y),
		NonLinear(ControllerRotSpeedNormalized.X, NonLinearMappingPower.Z)
		);

	ControllerRotSpeed_Rate = ControllerRotSpeed_NonLinear;
}

void UWeaponRotationSwayFragment::CalRotAcce(float DeltaSeconds)
{
	ControllerRotAcceleration = FVector2D((ControllerRotSpeed.X - PreControllerRotSpeed.X),
	                                      (ControllerRotSpeed.Y - PreControllerRotSpeed.Y)) / DeltaSeconds;
	PreControllerRotSpeed = ControllerRotSpeed;
}

void UWeaponRotationSwayFragment::CalculateRotSwayTarget(FVector& swayOffsetTarget, FRotator& swayRotTarget,bool bInvertRollPitch) const
{
	swayOffsetTarget = FVector(
		ControllerRotSpeed_Rate.X * SwayOffsetMulti.X,
		ControllerRotSpeed_Rate.X * SwayOffsetMulti.Y,
		ControllerRotSpeed_Rate.Z * SwayOffsetMulti.Z
	);
	
	//rotator构造函数顺序和蓝图显示顺序不一样，赋值能防止写参数时顺序混乱
	swayRotTarget.Roll = ControllerRotSpeed_Rate.X * SwayRotMulti.Roll;
	swayRotTarget.Pitch = ControllerRotSpeed_Rate.Y * SwayRotMulti.Pitch;
	swayRotTarget.Yaw = ControllerRotSpeed_Rate.Z * SwayRotMulti.Yaw;
	
	//Modify
	MouseXRotModify(swayRotTarget);
	
	// 调换Roll和Pitch轴
	if (bInvertRollPitch)
	{
		float tmpY=swayOffsetTarget.Y * -1.f;
		swayOffsetTarget.Y = swayOffsetTarget.X;
		swayOffsetTarget.X =tmpY;
		
		float tmpRoll=swayRotTarget.Roll;
		swayRotTarget.Roll = swayRotTarget.Pitch * -1.f;
		swayRotTarget.Pitch = tmpRoll;
	}
}

void UWeaponRotationSwayFragment::MouseXRotModify(FRotator& swayRotTarget) const
{
	//Yaw叠加Roll
	auto pitchAdditionMulti = ControllerRotSpeedNormalized.X < 0 ? PitchAdditionMultiWithYaw.X : PitchAdditionMultiWithYaw.Y;
	float pitch_Addition = ControllerRotSpeedNormalized.X * pitchAdditionMulti * FMath::Pow(
		1 - abs(ControllerRotSpeedNormalized.Y), PitchAdditionPower);
	swayRotTarget.Pitch += pitch_Addition;
}

void UWeaponRotationSwayFragment::CalPivotRotAndOffset(USkeletalMeshComponent* WeaponMesh,bool bInvertRollPitch,bool bDrawDebug)
{
	if (PivotMask == 0||WeaponMesh==nullptr)
	{
		return;
	}
	if (PivotSocket.IsNone() && PivotVector.IsZero())
	{
		return;
	}
	

	// 1) 中心坐标
	FVector PivotLoc = FVector::ZeroVector;
	FTransform pivotSocketTrans;
	// 1.1) 先尝试获取Socket，没有再获取Vector
	if (!PivotSocket.IsNone() && WeaponMesh)
	{
		pivotSocketTrans=WeaponMesh->GetSocketTransform(PivotSocket, RTS_Component);
		PivotLoc = pivotSocketTrans.GetLocation();
	}
	
	//debug
	PivotLoc += PivotVector;
	FTransform weaponTran= WeaponMesh->GetComponentTransform();
	
	if (bDrawDebug&&PivotLoc!=FVector::ZeroVector)
	{
		DrawDebugCoordinateSystem(GetWorld(),UKismetMathLibrary::TransformLocation(weaponTran,PivotLoc),UKismetMathLibrary::TransformRotation(weaponTran,pivotSocketTrans.Rotator()), 10.0f, false, -1.0f, 0, .4f);
	}

	

	// 2) 旋转量 
	FRotator pivotRot;
	bool bPivotRoll = PivotMask & 1 << static_cast<int32>(EPivotRotationMask::Roll);
	bool bPivotPitch = PivotMask & 1 << static_cast<int32>(EPivotRotationMask::Pitch);
	bool bPivotYaw = PivotMask & 1 << static_cast<int32>(EPivotRotationMask::Yaw);
	
	if (bInvertRollPitch)
	{
		bool tmp=bPivotPitch;
		bPivotPitch=bPivotRoll;
		bPivotRoll=tmp;
	}
	
	pivotRot.Roll = bPivotRoll ? SwayRot.Roll : 0;
	pivotRot.Pitch = bPivotPitch ? SwayRot.Pitch : 0;
	pivotRot.Yaw = bPivotYaw ? SwayRot.Yaw : 0;

	
	
	// 3) 计算
	if (PivotLoc != FVector::ZeroVector && !SwayRot.IsNearlyZero())
	{
		FVector targetLoc;
		FRotator targetRot;
		UWeaponSwayFunctionLibrary::RotateAroundPivot(PivotLoc, pivotRot, targetLoc, targetRot);
		SwayOffset.X = targetLoc.X;
		SwayOffset.Y = targetLoc.Y;
		SwayOffset.Z = targetLoc.Z;

		SwayRot.Roll = bPivotRoll ? targetRot.Roll : SwayRot.Roll;
		SwayRot.Pitch = bPivotPitch ? targetRot.Pitch : SwayRot.Pitch;
		SwayRot.Yaw = bPivotYaw ? targetRot.Yaw : SwayRot.Yaw;
	}
}

void UWeaponRotationSwayFragment::CalNoise(float DeltaSeconds,bool bInvertRollPitch,bool bHasInput)
{
	FRotator NoiseRotTarget=FRotator(0,0,0);
	// if (NoiseSpeed != 0.f)
	if (NoiseSpeed != 0.f)
	{
		NoiseTime += DeltaSeconds * NoiseSpeed;
		if (bHasInput)
		{
			if (abs(ControllerRotSpeed_Rate.X) >=.1f)
			{
				if (bInvertRollPitch)
				{
					NoiseRotTarget.Pitch=FMath::PerlinNoise1D(NoiseTime + 10.0f)*NoiseAmplitude.Roll;
				}
				else
				{
					NoiseRotTarget.Roll=FMath::PerlinNoise1D(NoiseTime + 10.0f)*NoiseAmplitude.Roll;
				}
			}
			if (abs(ControllerRotSpeed_Rate.Y)>=0.1f)
			{
				if (bInvertRollPitch)
				{
					NoiseRotTarget.Roll=FMath::PerlinNoise1D(NoiseTime + 10.0f)*NoiseAmplitude.Pitch;
				}
				else
				{
					NoiseRotTarget.Pitch=FMath::PerlinNoise1D(NoiseTime + 10.0f)*NoiseAmplitude.Pitch;
				}
			}
			if (abs(ControllerRotSpeed_Rate.Z)>=0.1f)
			{
				NoiseRotTarget.Yaw=FMath::PerlinNoise1D(NoiseTime + 20.0f) * NoiseAmplitude.Yaw;
			}
		}
		else
		{
			//方案2：interp
			NoiseRotTarget=FRotator::ZeroRotator;

		}
	
		NoiseRot= FMath::RInterpTo(NoiseRot,NoiseRotTarget,DeltaSeconds,NoiseInterpSpeed);
		SwayRot += NoiseRot;
	}

}

