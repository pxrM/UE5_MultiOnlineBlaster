// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "DataAsset_Anim_Walk.generated.h"

class UCurveFloat_ProceduralAnim;

USTRUCT(BlueprintType)
struct FWalkShakeParam
{
	GENERATED_BODY()
	/** 曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat_ProceduralAnim> Curve;
	
	/** 范围 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D Range=FVector2D(-1.f,1.f);
	
	/** 循环一次的时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Frequency=1.f;
	
	/** 曲线偏移(不受Time影响),减少重复感，不再需要手动调整曲线偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Offset=0.f;
public:
	FWalkShakeParam(FVector2D inRange=FVector2D::ZeroVector,float inTime=1.f):Range(inRange),Frequency(inTime)
	{}
};
/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UDataAsset_Anim_Walk : public UDataAsset_ProceduralAnim
{
	GENERATED_BODY()
public:
	//Todo: 思考多种状态下动态设置PlayRate的方式，而不是写的MaxSpeed
	/** 标准速度：playRate=1 LagMove=1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StandardWalkSpeed=600.f;
	
#pragma region shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	bool bEnableShake=true;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	// bool ShakeInterpOnlyStop=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	bool bShakeUseLowpass=true;
	
	/** 低通滤波阈值 default=0.05s 越低->越灵敏 越高->越迟钝/更平滑，主要用于Lag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	float ShakeLowpassThreshold=.05f;
	
	/** 晃动强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	float ShakeStrength=1.f;
	
	/** 晃动频率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	float ShakeFrequency=1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	FWalkShakeParam Shake_X=FVector2D(-.4f,.4f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	FWalkShakeParam Shake_Y=FVector2D(0.f,0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	FWalkShakeParam Shake_Z=FVector2D(-.35f,.2f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Shake")
	FWalkShakeParam Shake_Rot=FVector2D(-1.f,1.f);
#pragma endregion
	
#pragma region lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag")
	bool bEnableLag=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag")
	bool bLagUseLowpass=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag",meta=(EditCondition=bLagUseLowpass))
	float LagLowpassThreshold=.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag")
	float LagMaxLength=4.f;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag")
	// bool LagInterpOnlyStop=true;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag")
	// float LagInterpSpeed=6.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Loc")
	FVector LagOffset_Right=FVector(1.f,0.f,0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Loc")
	FVector LagOffset_Forward=FVector(0.f,1.f,0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Loc")
	FVector LagOffset_Up=FVector(0.f,0.f,1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Rot")
	FRotator LagRot_Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Rot")
	FRotator LagRot_Forward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Rot")
	FRotator LagRot_Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Length")
	float LagOffsetLengthLowpassThreshold=.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|Length")
	FVector LagOffset_Length=FVector(0.f,0.f,-1.f);
	
#pragma endregion
#pragma region lag_spring
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|SpringInterp")
	bool bEnableSpringInterp=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|SpringInterp",meta=(EditCondition=bEnableSpringInterp,ClampMin=0))
	float Stiffness=100.f;
	
	/** .7 .8 .9 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|SpringInterp",meta=(EditCondition=bEnableSpringInterp,ClampMin=0,ClampMax=2))
	float CriticalDampingFactor=.7f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|SpringInterp",meta=(EditCondition=bEnableSpringInterp,ClampMin=0,ClampMax=2))
	float Mass=1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Lag|SpringInterp",meta=(EditCondition=bEnableSpringInterp,ClampMin=0,ClampMax=1))
	float TargetVelocityAmount=1.f;
#pragma endregion
};
