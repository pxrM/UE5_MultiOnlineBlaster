// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"
#include "ProcedualAnim/FragmentDatas/DataAsset_Anim_Wiggle.h"
#include "PAnimFragment_Wiggle.generated.h"

/**
 * Todo: 将动画偏移提取为独立、节省的数据参数，哪怕是先固定动画，再手动用曲线模拟
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UPAnimFragment_Wiggle : public UProceduralAnimFragment
{
	GENERATED_BODY()
public:
	UPAnimFragment_Wiggle();
	virtual void Initialize(UFPSProceduralAnimComp* InOwnerComp) override;
	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveSignal(const FGameplayTag& Signal) override;

	virtual FORCEINLINE UDataAsset_ProceduralAnim* GetData() override { return Data; }
	virtual bool SetData(UDataAsset_ProceduralAnim* InData) override;
protected:
	void StartAnimation(FGameplayTag tag);
	
	FTransform GetAnimTrans(UAnimSequence* InAnim,float InTime) const;
	
	UAnimSequence* GetBasePose(UAnimSequence* anim) const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	TObjectPtr<UDataAsset_Anim_Wiggle> Data;
	
	//PreAnim
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimSequence> PreAnim;
	UPROPERTY(BlueprintReadWrite)
	float PreAnimTime=0.f;
	UPROPERTY(BlueprintReadWrite)
	float CurBlendOutTime=0.f;
	UPROPERTY(BlueprintReadWrite)
	bool bIsPlaying_PreAnim=false;
	
	//CurAnim
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimSequence> CurAnim;
	UPROPERTY(BlueprintReadWrite)
	float CurAnimTime=0.f;
	UPROPERTY(BlueprintReadWrite)
	float CurBlendInTime=0.f;
	UPROPERTY(BlueprintReadWrite)
	bool bIsPlaying_CurAnim=false;
};
