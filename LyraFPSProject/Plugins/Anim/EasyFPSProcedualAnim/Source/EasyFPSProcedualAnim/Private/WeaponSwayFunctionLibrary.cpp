// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSwayFunctionlibrary.h"
#include "WeaponRotationSway/WeaponRotationSwayData.h"

FRotator UWeaponSwayFunctionLibrary::RotatorSpringInterp(const FRotator& Current, const FRotator& Target,
                                                         FRotatorSpringState& State, float Stiffness, float Damping, float DeltaTime, float Mass, FRotator MaxRot,float SwayRotMaxDeltaSeconds)
{
	DeltaTime=DeltaTime>SwayRotMaxDeltaSeconds?SwayRotMaxDeltaSeconds:DeltaTime;
	// Convert to vector for simple integration (Pitch, Yaw, Roll)
	FVector cur(Current.Pitch, Current.Yaw, Current.Roll);
	FVector targ(Target.Pitch, Target.Yaw, Target.Roll);


	// Semi-implicit Euler integration
	FVector acc = (targ - cur) * Stiffness - State.Velocity * Damping;
	acc = acc / FMath::Max(Mass, KINDA_SMALL_NUMBER);
	State.Velocity += acc * DeltaTime;
	FVector next = cur + State.Velocity * DeltaTime;


	FRotator nextRot = FRotator(next.X, next.Y, next.Z);
	if (!MaxRot.IsZero())
	{
		nextRot.Pitch = FMath::Clamp(nextRot.Pitch, -MaxRot.Pitch, MaxRot.Pitch);
		nextRot.Yaw = FMath::Clamp(nextRot.Yaw, -MaxRot.Yaw, MaxRot.Yaw);
		nextRot.Roll = FMath::Clamp(nextRot.Roll, -MaxRot.Roll, MaxRot.Roll);
	}
	return nextRot;
}

/**
 * 绕某个 Pivot 点做旋转（任意旋转），返回变换后的位置与旋转
 * 原理：T = Pivot + R * (Pos - Pivot)
 */
void UWeaponSwayFunctionLibrary::RotateAroundPivot(FVector Pivot, const FRotator& DeltaRot, FVector& OutPosition,
	FRotator& OutRotation, const FVector InPosition, const FRotator InRotation)
{
	// Step 1: 相对 Pivot 的偏移
	FVector RelPos = InPosition - Pivot;

	// Step 2: 构建旋转 Quat
	FQuat QuatRot = DeltaRot.Quaternion();

	// Step 3: 应用旋转
	FVector RotatedRelPos = QuatRot * RelPos;

	// Step 4: 恢复到世界位置
	OutPosition = Pivot + RotatedRelPos;

	// Step 5: 应用旋转叠加
	OutRotation = (FRotationMatrix(InRotation).ToQuat() * QuatRot).Rotator();
}

FRotator UWeaponSwayFunctionLibrary::MicroSwayNoise(float &NoiseTime,float DeltaTime,float NoiseAmount, float NoiseSpeed)
{
	NoiseTime += DeltaTime * NoiseSpeed;
	float NoiseYaw = (FMath::PerlinNoise1D(NoiseTime) - 0.5f) * NoiseAmount;
	float NoisePitch = (FMath::PerlinNoise1D(NoiseTime + 100.f) - 0.5f) * NoiseAmount;

	return FRotator(NoisePitch, NoiseYaw, 0);
}
