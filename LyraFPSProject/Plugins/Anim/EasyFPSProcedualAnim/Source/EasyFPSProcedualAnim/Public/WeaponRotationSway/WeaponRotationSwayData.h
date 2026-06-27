// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WeaponRotationSwayData.generated.h"


// Rotator spring state
USTRUCT(BlueprintType)
struct EASYFPSPROCEDUALANIM_API FRotatorSpringState
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector; // pitch, yaw, roll velocity
};


