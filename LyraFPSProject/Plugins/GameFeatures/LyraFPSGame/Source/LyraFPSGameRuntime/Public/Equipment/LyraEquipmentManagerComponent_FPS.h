// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "LyraEquipmentManagerComponent_FPS.generated.h"


class ULyraEquipmentDefinition_FPS;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraEquipmentManagerComponent_FPS : public ULyraEquipmentManagerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraEquipmentManagerComponent_FPS(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	FLyraEquipmentList* GetEquipmentList(){return &EquipmentList;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ULyraEquipmentDefinition_FPS* GetDefinitionByInstance(ULyraEquipmentInstance* InstanceType);

};
