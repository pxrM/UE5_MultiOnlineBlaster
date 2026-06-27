// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraTaggedActor.h"
#include "LyraTaggedActor_FPS.generated.h"

UCLASS()
class LYRAFPSGAMERUNTIME_API ALyraTaggedActor_FPS : public ALyraTaggedActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALyraTaggedActor_FPS();


	//让Tags查询吸附到的组件
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
