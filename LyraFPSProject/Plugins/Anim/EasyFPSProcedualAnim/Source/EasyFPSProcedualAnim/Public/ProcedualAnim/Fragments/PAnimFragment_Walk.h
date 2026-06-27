// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"
#include "ProcedualAnim/FragmentDatas/DataAsset_Anim_Walk.h"
#include "WeaponRotationSway/WeaponRotationSwayFragment_SpringInterp.h"
#include "PAnimFragment_Walk.generated.h"

USTRUCT(BlueprintType)
struct FProceduralWalkValues
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Alpha = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
};

class UDataAsset_Anim_Walk;
/**
 * 
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UPAnimFragment_Walk : public UProceduralAnimFragment
{
	GENERATED_BODY()
public:
	UPAnimFragment_Walk();
	virtual void Initialize(UFPSProceduralAnimComp* InOwnerComp) override;
	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveSignal(const FGameplayTag& Signal) override;

	virtual FORCEINLINE UDataAsset_ProceduralAnim* GetData() override { return Data; }
	virtual bool SetData(UDataAsset_ProceduralAnim* InData) override;
protected:
	void RefreshShakeParams();
	void UpdateShakes(float DeltaTime);
	
	//直接将速度映射为偏移：精确、无回弹和动态效果
	void UpdateLag_RealTime(float DeltaTime);
protected:

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataAsset_Anim_Walk> Data;
	
	TArray<FWalkShakeParam*> ShakeParams;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FProceduralWalkValues> ShakeValues;
	
	//Todo: 实现有问题，应该是横向移动绑定+纵向移动绑定一套
	UPROPERTY(BlueprintReadWrite)
	FVector LagAlpha;
	/** 转为Length提供的LagAlpha，其不受具体插值影响 */
	UPROPERTY(BlueprintReadWrite)
	FVector LagAlpha_Length;
	UPROPERTY(BlueprintReadWrite)
	FVector LagAlphaTarget;
	UPROPERTY(BlueprintReadWrite)
	FVector LagOffset;
	UPROPERTY(BlueprintReadWrite)
	FRotator LagRot;
	UPROPERTY(BlueprintReadOnly)
	float AnimPlaySpeedAlpha=0.f;
	
	//Spring Interp
	FVectorSpringState LagSpringState;
	FVectorSpringState LagSpringState_Rot;
};
