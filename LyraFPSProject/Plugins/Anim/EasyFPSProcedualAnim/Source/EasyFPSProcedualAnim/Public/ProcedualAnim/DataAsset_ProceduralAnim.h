// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_ProceduralAnim.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class EASYFPSPROCEDUALANIM_API UDataAsset_ProceduralAnim : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** 根据不同目标存放不同的倍率，如相机倍率较低 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta=(Categories="ProceduralAnim.Fragment.Target"))
	TMap<FGameplayTag,float> TargetOffsetRate;
#if WITH_EDITOR
protected:
	// 在你的 Actor 类头文件中声明
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
