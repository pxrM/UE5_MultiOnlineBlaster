// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponRotationSwayData.h"
#include "WeaponRotationSwayFragment.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponRotationSwayFragment_SpringInterp.generated.h"



/**
 * 
 */
UCLASS(Blueprintable,BlueprintType,Abstract)
class EASYFPSPROCEDUALANIM_API UWeaponRotationSwayFragment_SpringInterp : public UWeaponRotationSwayFragment
{
	GENERATED_BODY()
public:
	UWeaponRotationSwayFragment_SpringInterp();
	virtual void WeaponRotationSwayTick_Implementation(float DeltaSeconds,FRotator CurrtRot,bool bHasInput,USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=false,bool bDebug=false) override;

protected:
	// spring params
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pos")
	float Pos_Stiffness = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pos")
	float Pos_Damping = 6.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pos")
	float Pos_Mass = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rot")
	float Rot_Stiffness = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rot")
	float Rot_Damping = 6.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rot")
	float Rot_Mass = 1.f;

	
	//Clamp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pos")
	float MaxOffsetLength = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rot")
	FRotator MaxRot = FRotator(20.f,20.f,20.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pos",meta=(ClampMin=0,ClampMax=1))
	float TargetVelocityAmount = 0.f;
	
	// spring states
	FVectorSpringState PosSpringState;
	FRotatorSpringState RotSpringState;
	
#pragma region RotSpringInterp
protected:
	/** 旋转SpringInterp最大间隔时间，过大的间隔时间会导致无限加速bug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rot")
	float SwayRotMaxDeltaSeconds=.05f;
#pragma endregion

};
