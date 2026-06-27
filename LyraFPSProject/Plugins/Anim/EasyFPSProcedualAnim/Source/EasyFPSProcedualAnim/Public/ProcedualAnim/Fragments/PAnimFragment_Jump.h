// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcedualAnim/FragmentDatas/DataAsset_Anim_Jump.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"
#include "PAnimFragment_Jump.generated.h"

class UCurveFloat_ProceduralAnim;
class UCharacterMovementComponent;
class UFPSProceduralAnimComp;

UENUM(BlueprintType)
enum class EJumpState : uint8
{
	ENone = 0,
	EJump,
	EInAir,
	ELand
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class EASYFPSPROCEDUALANIM_API UPAnimFragment_Jump : public UProceduralAnimFragment
{
	GENERATED_BODY()

public:
	UPAnimFragment_Jump();
	virtual void Initialize(UFPSProceduralAnimComp* InOwnerComp) override;
	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveSignal(const FGameplayTag& Signal) override;

	virtual FORCEINLINE UDataAsset_ProceduralAnim* GetData() override { return Data; }
	virtual bool SetData(UDataAsset_ProceduralAnim* InData) override;

protected:
	// Jump->InAir->Land
	UFUNCTION()
	void OnStartJump();
	void InAirCheck();
	UFUNCTION()
	void OnLanded(const FHitResult& Hit);

	//Dip
	void Dip(UCurveFloat_ProceduralAnim* Curve, float Time = 1.f, float Target = 1.f,
	         EJumpState State = EJumpState::ENone, float strength = 1.f);
	void DipUpdate(float DeltaSeconds);

	//Bind
	void AutoBindLandEvent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties), Category="Default")
	TObjectPtr<UDataAsset_Anim_Jump> Data;

#pragma region Dip

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	EJumpState DipState = EJumpState::ENone;
	float DipTotalTime = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float CurDipTime = 0.f;
	//Land混合开始时间
	float TotalBlendTime = -1.f;


	TSoftObjectPtr<UCurveFloat_ProceduralAnim> DipCurve;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float DipStrength = 1.f;
	//当曲线溢出1的时候，回归时才开始混合
	bool bCurveIsOverflow = false;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float DipStrengthBlendAlpha = 0.f;

	float DipStart;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float DipTarget;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float DipAlpha = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Dip")
	float CurDipOffset = 0.f;

#pragma endregion
};
