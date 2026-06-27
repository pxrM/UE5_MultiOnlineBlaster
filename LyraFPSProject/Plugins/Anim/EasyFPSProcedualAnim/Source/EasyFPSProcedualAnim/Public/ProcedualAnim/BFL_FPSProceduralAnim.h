// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_FPSProceduralAnim.generated.h"

class UAnimSequence;

/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UBFL_FPSProceduralAnim : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure)
	static FTransform GetBoneTrans_Pose(UAnimSequence* Animation,FName BoneName,float Time=0.f);
	
};
