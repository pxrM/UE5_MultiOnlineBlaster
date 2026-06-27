// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "SkeletalMeshComponent_FPS.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API USkeletalMeshComponent_FPS : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	USkeletalMeshComponent_FPS();
	// USkeletalMeshComponent_FPS(FObjectInitializer& InitializerModule = FObjectInitializer::Get());
};
