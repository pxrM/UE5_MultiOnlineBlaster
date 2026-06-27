// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "DataAsset_Anim_Wiggle.generated.h"

class UCurveFloat_ProceduralAnim;
/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UDataAsset_Anim_Wiggle : public UDataAsset_ProceduralAnim
{
	GENERATED_BODY()
public:
	//Todo: 同时触发Crouch和Aim时，是否需要同时播放
	/** 事件触发时要调用的动画集合 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(Categories="ProceduralAnim.Signal"))
	TMap<FGameplayTag, TObjectPtr<UAnimSequence>> Anims;
	
	/** If Anim is Additive, no configuration is required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> DefaultBasePose;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName="ik_hand_gun";
	
	/** 动画混入时间 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(ClampMin=0))
	float BlendInTime=.25f;
	
	/** 动画混出时间 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(ClampMin=0))
	float BlendOutTime=.25f;
	
	// Todo: 是否需要更精细的曲线控制
	// UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Wiggle)
	// FMontageBlendSettings MontageBlendInSettings;
};
