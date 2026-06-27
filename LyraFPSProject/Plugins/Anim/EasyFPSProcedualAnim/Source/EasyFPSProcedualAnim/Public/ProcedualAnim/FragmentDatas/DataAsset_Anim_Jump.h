// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "DataAsset_Anim_Jump.generated.h"

class UCurveFloat_ProceduralAnim;
/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UDataAsset_Anim_Jump : public UDataAsset_ProceduralAnim
{
	GENERATED_BODY()
public:

	//Jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	TObjectPtr<UCurveFloat_ProceduralAnim> JumpCurve;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Jump")
	float JumpDipTime=.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpTargetZ=-1.f;
	
	//InAir
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InAir")
	TObjectPtr<UCurveFloat_ProceduralAnim> InAirCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InAir")
	float MaxInAirTime=1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InAir")
	float InAirTarget=2.f;
	
	//Land
	//Todo：低速时采用另一套曲线用以保证效果流畅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land")
	TObjectPtr<UCurveFloat_ProceduralAnim> LandCurve;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Land")
	float LandDipTime=1.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Land")
	float LandDipStrength=1.f;
	//Land-Drop
	//超过MaxJumpZ后的强度乘数
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Land|Drop",meta=(ClampMin=0.f))
	float LandDropStrengthRate=2.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Land|Drop",meta=(ClampMin=0.f))
	float LandDropTimeRate=2.f;
	/** f(x)=1-t/(x+t) */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Land|Drop",meta=(ClampMin=0.001f))
	float LandDropTimeCurvePower=1.f;

	//Bind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dip")
	bool bAutoBindCharacterLandEvent=true;
};
