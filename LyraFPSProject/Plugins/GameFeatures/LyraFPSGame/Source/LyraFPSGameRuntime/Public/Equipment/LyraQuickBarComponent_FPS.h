// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "LyraQuickBarComponent_FPS.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraQuickBarComponent_FPS : public ULyraQuickBarComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraQuickBarComponent_FPS(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual ULyraEquipmentManagerComponent* FindEquipmentManager() const override;
};
