// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "WeaponRotationSwayFragment.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class EPivotRotationMask : uint8
{
	Roll=0,
	Pitch=1,
	Yaw=2
};

/**
 * 
 */

UCLASS( DefaultToInstanced, EditInlineNew,BlueprintType, Abstract,PrioritizeCategories=("WeaponRotSway","Input","Target"))
class EASYFPSPROCEDUALANIM_API UWeaponRotationSwayFragment : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	void WeaponRotationSwayTick(float DeltaSeconds,FRotator CurrtRot,bool bHasInput,USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=true,bool bDebug=false);
	virtual void WeaponRotationSwayTick_Implementation(float DeltaSeconds,FRotator CurrtRot,bool bHasInput,USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=true,bool bDebug=false){}
protected:
	
	UPROPERTY(BlueprintReadWrite, Category = "WeaponRotaionSway")
	FVector SwayOffset;
	
	UPROPERTY(BlueprintReadWrite, Category = "WeaponRotaionSway")
	FRotator SwayRot;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FRotator ControlLastRot;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector2D ControllerRotSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector2D ControllerRotSpeedNormalized;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector ControllerRotSpeed_NonLinear;
	
	/** 最终用于计算的数值 */
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector ControllerRotSpeed_Rate;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector2D PreControllerRotSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "WeaponRotaionSway")
	FVector2D ControllerRotAcceleration;
	
#pragma region Calculate
public:
	void CalculateControllerRotParams(float DeltaSeconds,FRotator CurrtRot, bool bUseLowpass);
	void ProcessRotSpeed(const FVector2D& originRotSpeed,float DeltaSeconds, bool bUseLowpass);
	UFUNCTION(BlueprintCallable)
	void CalRotAcce(float DeltaSeconds);
	
	void CalculateRotSwayTarget(FVector& swayOffsetTarget,FRotator& swayRotTarget,bool bInvertRollPitch) const;
	void MouseXRotModify(FRotator& swayRotTarget) const;

	// void ControllerRotAddNoise(float DeltaSeconds,FRotator CurrtRot, bool bUseLowpass);
#pragma endregion
	
#pragma region Params
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "WeaponRotSway")
	FGameplayTagContainer Tags;
	
	/** RotSpeedNormalized(0-1) = RotSpeedNormalized^CurvePower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input",meta=(ClampMin=0,ClamMax=1))
	FVector NonLinearMappingPower = FVector::OneVector;
	
	// normalization and clamps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float FiducialRotSpeed = 1080.f; // deg/s considered full
	
	/** 是否对deltaRot使用低通滤波降低瞬时摆动幅度 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Input", meta = (InlineEditConditionToggle))
	bool bUseInputLowpass=false;
	
	/** lowpass：时间间隔<0.1时，执行0-1映射 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Input",meta = (EditCondition = "bUseInputLowpass"))
	float InputLowpassThreshold=.1f;
	
	//Multi
	UPROPERTY()
	bool bUsePos=true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target",meta=(EditCondition = "bUsePos",EditConditionHides,HideEditConditionToggle))
	FVector SwayOffsetMulti;
	
	UPROPERTY()
	bool bUseRot=true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target",meta=(EditCondition = "bUseRot",EditConditionHides,HideEditConditionToggle))
	FRotator SwayRotMulti;
	
	

#pragma endregion


	
	
#pragma region YawPivotRotAndOffset
public:
	/** Yaw旋转中心点计算 */
	void CalPivotRotAndOffset(USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=true, bool bDrawDebug=false);

protected:
	UPROPERTY(EditAnywhere, Category="PostProcess|YawPivot",meta = (Bitmask, BitmaskEnum="/Script/EasyFPSProcedualAnim.EPivotRotationMask",CategoryPriority =10))
	uint8 PivotMask;
	
	/** The coordinates of the Yaw rotation center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess|YawPivot")
	FVector PivotVector = FVector::ZeroVector;
	/** The socket of the Yaw rotation center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess|YawPivot")
	FName PivotSocket;
	

	
#pragma endregion
	
#pragma region Noise
public:
	void CalNoise(float DeltaSeconds,bool bInvertRollPitch,bool bHasInput);
	
protected:
	/** 柏林噪声速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f,CategoryPriority=10),Category="PostProcess|Noise")
	float NoiseSpeed=0.f;
	
	/** 柏林噪声强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f),Category="PostProcess|Noise")
	FRotator NoiseAmplitude=FRotator(0.05,0.05,0.1);
	
	/** 最终Noise大小 */
	UPROPERTY(BlueprintReadWrite,Category="PostProcess|Noise")
	FRotator NoiseRot;
	
	/** 储存时间 */
	UPROPERTY(BlueprintReadWrite,Category="PostProcess|Noise")
	float NoiseTime=0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f), Category="PostProcess|Noise")
	float NoiseInterpSpeed=6.f;

#pragma endregion
	
#pragma region PitchAdd
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess|PitchAdd")
	FVector2D PitchAdditionMultiWithYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,AdvancedDisplay, Category = "PostProcess|PitchAdd")
	float PitchAdditionPower = 2.f;
#pragma endregion
};
