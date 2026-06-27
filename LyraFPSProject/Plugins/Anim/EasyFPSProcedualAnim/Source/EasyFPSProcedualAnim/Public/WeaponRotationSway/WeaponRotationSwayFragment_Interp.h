// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponRotationSwayData.h"
#include "WeaponRotationSwayFragment.h"
#include "WeaponRotationSwayFragment_Interp.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType,Abstract)
class EASYFPSPROCEDUALANIM_API UWeaponRotationSwayFragment_Interp : public UWeaponRotationSwayFragment
{
	GENERATED_BODY()
public:
	virtual void WeaponRotationSwayTick_Implementation(float DeltaSeconds,FRotator CurrtRot,bool bHasInput,USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=false,bool bDebug=false) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target|Interp")
	float InterpSpeed_Pos_Input = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target|Interp")
	float InterpSpeed_Pos_NoInput = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target|Interp")
	float InterpSpeed_Rot_Input = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target|Interp")
	float InterpSpeed_Rot_NoInput = 25.f;
};
