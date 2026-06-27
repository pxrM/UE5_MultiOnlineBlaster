// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/LyraCameraComponent.h"
#include "Camera/LyraCameraMode.h"
#include "LyraFPSCameraComponent.generated.h"

USTRUCT(BlueprintType)
struct LYRAFPSGAMERUNTIME_API FCameraShakeRatioPair
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShakeRatio=.7f;
	
	// FCameraShakeRatioPair(FString ID)
	// :ID(ID)
	// {
	// 	
	// }
	
	FCameraShakeRatioPair(FString ID="",float ShakeRatio=.7f)
		:ID(ID),ShakeRatio(ShakeRatio)
	{
		
	}
	
	bool operator==(const FCameraShakeRatioPair& Other) const
	{
		return ID==Other.ID;
	}
};

UCLASS()
class LYRAFPSGAMERUNTIME_API ULyraFPSCameraModeStack : public ULyraCameraModeStack
{
	GENERATED_BODY()

public:
	void BlendFirstPersonParameters(float& OutFirstPersonFOV, float& OutFirstPersonScale, bool& bOutUseFirstPersonParameters) const;
	ULyraCameraMode* GetTopCameraMode() const;
};

/**
 * 
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ULyraFPSCameraComponent : public ULyraCameraComponent
{
	GENERATED_BODY()
public:
	ULyraFPSCameraComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
	ULyraCameraMode* GetCurCameraMode();
public:
	UFUNCTION(BlueprintCallable)
	void AddCameraShakeRatio(FString ID,float ShakeRatio);
	
	UFUNCTION(BlueprintCallable)
	void RemoveCameraShakeRatio(FString ID);
protected:
	FTransform GetCameraTransform(UAnimSequence*  InitSequence) const;

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
protected:
	FTransform InitCameraTransform;
	//应该移到CameraMode中
	UPROPERTY(BlueprintReadWrite)
	TArray<FCameraShakeRatioPair> headCameraShadeRatioStack;
};
