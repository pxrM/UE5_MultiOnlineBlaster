// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "DataAsset_Anim_Crouch.generated.h"

class UCurveFloat_ProceduralAnim;
/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UDataAsset_Anim_Crouch : public UDataAsset_ProceduralAnim
{
	GENERATED_BODY()

public:
	/** 混合时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.1f),Category="Blend")
	float BlendTime = .5f;
	/** 混合曲线、暂时只支持自定义曲线 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Blend")
	TObjectPtr<UCurveFloat_ProceduralAnim> BlendCurve;
	
	/** 位置偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Pose")
	FVector Offset = FVector(-2.f,0.f,-2.f);
	
	/** 位置旋转 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Pose")
	FRotator Rotator = FRotator(-5.f,0.f,0.f);
	
	/** CrouchPose，用于自动采集偏移量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Pose")
	TObjectPtr<UAnimSequence> Pose_Crouch;
	
	/** If Pose Crouch is Additive, no configuration is required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Pose")
	TObjectPtr<UAnimSequence> Pose_Base;
	
	//Camera
	/** 是否采用Character的CrouchedEyeHeight作为Camera高度依据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Camera")
	bool bUseCrouchedEyeHeight = true;
	/** 相机高度偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Camera")
	float CameraHeightOffset = 0.f;
};
