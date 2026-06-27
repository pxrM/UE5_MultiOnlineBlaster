// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/LyraCameraMode.h"
#include "LyraFPSCameraMode.generated.h"

class UCurveVector;
/**
 * 
 */
UCLASS(Blueprintable)
class LYRAFPSGAMERUNTIME_API ULyraFPSCameraMode : public ULyraCameraMode
{
	GENERATED_BODY()
public:
	ULyraFPSCameraMode();
	
	
	virtual void OnActivation() override;
	
protected:
	virtual void UpdateView(float DeltaTime) override;
	
protected:
	TSoftObjectPtr<class ALyraFPSCharacter> TargetCharacter;
	
	//将这些分区都做成Object实例功能，要哪个添哪个
#pragma region HeadShake 
public:
	float GetHeadCameraShakeAdditiveRatio() const;
public:
	/** 相机晃动比例 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPS")
	float headCameraShadeRatio=1.f;

	/** 相机晃动动画骨骼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPS")
	FName  CameraShakeBoneName="Head";

	/** 用于计算相对位置的初始Pose */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPS")
	TObjectPtr<UAnimSequence>  InitSequence;
	
	/** 用于计算的坐标系 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPS")
	TEnumAsByte<ERelativeTransformSpace>  TransformSpace=RTS_Actor;
#pragma endregion
	
#pragma region FirstPerson
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View|FirstPerson",meta=(ClampMin=0.f,ClampMax=170.f))
	float FirstPersonFieldOfView=97.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View|FirstPerson",meta=(ClampMin=0.001f,ClampMax=1.f))
	float FirstPersonScale=.5f;
#pragma endregion
};


