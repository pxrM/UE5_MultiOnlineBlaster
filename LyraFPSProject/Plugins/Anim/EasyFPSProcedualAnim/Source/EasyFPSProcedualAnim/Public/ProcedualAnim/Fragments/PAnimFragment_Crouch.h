// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"
#include "PAnimFragment_Crouch.generated.h"

class UDataAsset_Anim_Crouch;
/**
 * Crouch
 * Todo: 让下蹲Pose支持Anim
 */
UCLASS()
class EASYFPSPROCEDUALANIM_API UPAnimFragment_Crouch : public UProceduralAnimFragment
{
	GENERATED_BODY()
public:
	UPAnimFragment_Crouch();
	virtual void Initialize(UFPSProceduralAnimComp* InOwnerComp) override;
	virtual void Tick(float DeltaTime) override;

	virtual FVector GetOffset(FGameplayTag TargetType = FGameplayTag()) override;
	virtual void ReceiveSignal(const FGameplayTag& Signal) override;
	virtual UDataAsset_ProceduralAnim* GetData() override;
	virtual bool SetData(UDataAsset_ProceduralAnim* InData) override;
protected:
	/** 获得目标偏移 */
	void InitCameraTargetOffset();
	void InitPoseTargetTransform();
	
	void StartCrouch(bool bNewCrouch);
	void UpdateCrouchAlpha(float DeltaTime);
	
	UAnimSequence* GetBasePose(UAnimSequence* anim) const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crouch")
	TObjectPtr<UDataAsset_Anim_Crouch> Data;
	/** 相机高度 */
	UPROPERTY(BlueprintReadOnly)
	float CameraHeight=0.f;
	/** 蹲下时的Pose信息 */
	UPROPERTY(BlueprintReadOnly)
	FTransform CrouchPoseTransform;

	//当前混合Alpha
	UPROPERTY(BlueprintReadOnly, Category = "Crouch")
	float OffsetBlendPct = 0.f;
	float BlendTimeRate=1.f;
	UPROPERTY(BlueprintReadOnly, Category = "Crouch")
	float InitialAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "Crouch")
	float TargetAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "Crouch")
	bool bBlending=false;
};
