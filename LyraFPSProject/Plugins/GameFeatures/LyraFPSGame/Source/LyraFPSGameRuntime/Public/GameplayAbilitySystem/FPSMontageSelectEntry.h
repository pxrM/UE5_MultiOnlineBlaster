// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FPSMontageSelectEntry.generated.h"


/**
 * 用于
 *  */
USTRUCT(BlueprintType)
struct FFPSMontageSelectEntry
{
	GENERATED_BODY()

	// Cosmetic tags required (all of these must be present to be considered a match)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;
	
	// Layer to apply if the tag matches
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FPSAnim;
};

