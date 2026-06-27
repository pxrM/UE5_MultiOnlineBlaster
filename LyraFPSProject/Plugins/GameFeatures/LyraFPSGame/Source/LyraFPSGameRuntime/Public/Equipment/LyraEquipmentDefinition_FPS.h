// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraEquipmentDefinition.h"
#include "LyraEquipmentDefinition_FPS.generated.h"

/**
 * 
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ULyraEquipmentDefinition_FPS : public ULyraEquipmentDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category=Equipment,meta=(DisplayAfter="ActorsToSpawn"))
	TArray<FLyraEquipmentActorToSpawn> ActorsToSpawn_Local;
};
