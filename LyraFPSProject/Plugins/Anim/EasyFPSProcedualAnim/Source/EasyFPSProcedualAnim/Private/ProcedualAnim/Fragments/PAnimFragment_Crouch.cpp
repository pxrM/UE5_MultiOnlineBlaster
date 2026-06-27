// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/Fragments/PAnimFragment_Crouch.h"

#include "FPSAnimGameplayTags.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProcedualAnim/BFL_FPSProceduralAnim.h"
#include "ProcedualAnim/CurveFloat_ProceduralAnim.h"
#include "ProcedualAnim/FragmentDatas/DataAsset_Anim_Crouch.h"


UPAnimFragment_Crouch::UPAnimFragment_Crouch()
{
	Tag = FPSAnimGameplayTags::Fragment_Crouch;
}

void UPAnimFragment_Crouch::Initialize(UFPSProceduralAnimComp* InOwnerComp)
{
	Super::Initialize(InOwnerComp);

	if (Data)
	{
		InitPoseTargetTransform();
		InitCameraTargetOffset();
	}
}

void UPAnimFragment_Crouch::Tick(float DeltaTime)
{
	

	UpdateCrouchAlpha(DeltaTime);
	if (Data)
	{
		Offset = (Data->Offset + CrouchPoseTransform.GetLocation()) * Alpha;
		Rot = (Data->Rotator + CrouchPoseTransform.Rotator()) * Alpha;
	}
	Super::Tick(DeltaTime);
}

FVector UPAnimFragment_Crouch::GetOffset(FGameplayTag TargetType)
{
	if (TargetType == FPSAnimGameplayTags::Fragment_Target_Camera)
	{
		return FVector(0.f, 0.f, Alpha * CameraHeight);
	}
	return Super::GetOffset(TargetType);
}

void UPAnimFragment_Crouch::ReceiveSignal(const FGameplayTag& Signal)
{
	if (Signal == FPSAnimGameplayTags::Signal_Crouch)
	{
		StartCrouch(true);
	}
	if (Signal == FPSAnimGameplayTags::Signal_UnCrouch)
	{
		StartCrouch(false);
	}
}

UDataAsset_ProceduralAnim* UPAnimFragment_Crouch::GetData()
{
	return Data;
}

bool UPAnimFragment_Crouch::SetData(UDataAsset_ProceduralAnim* InData)
{
	if (auto newData = Cast<UDataAsset_Anim_Crouch>(InData))
	{
		Data = newData;
		if (OwnerComp)
		{
			InitPoseTargetTransform();
			InitCameraTargetOffset();
		}
		return true;
	}
	return false;
}

void UPAnimFragment_Crouch::InitCameraTargetOffset()
{
	CameraHeight = 0.f;
	if (!Data || !OwnerChar)
	{
		return;
	}

	if (Data->bUseCrouchedEyeHeight)
	{
		const ACharacter* OwnerCharCDO = OwnerChar->GetClass()->GetDefaultObject<ACharacter>();
		CameraHeight = OwnerCharCDO->CrouchedEyeHeight - OwnerCharCDO->BaseEyeHeight;
	}
	CameraHeight+=Data->CameraHeightOffset;
}

void UPAnimFragment_Crouch::InitPoseTargetTransform()
{
	CrouchPoseTransform = FTransform::Identity;
	if (!Data)
	{
		return;
	}

	//通过Pose获得Crouch偏移量
	if ( Data->Pose_Crouch)
	{
		if (UAnimSequence* basePose = GetBasePose(Data->Pose_Crouch))
		{
			FTransform baseTrans = UBFL_FPSProceduralAnim::GetBoneTrans_Pose(
			basePose,TEXT("ik_hand_gun"));
			FTransform crouchTrans = UBFL_FPSProceduralAnim::GetBoneTrans_Pose(
				Data->Pose_Crouch,TEXT("ik_hand_gun"));
			CrouchPoseTransform.SetLocation(crouchTrans.GetLocation() - baseTrans.GetLocation());
			CrouchPoseTransform.SetRotation(
				UKismetMathLibrary::NormalizedDeltaRotator(crouchTrans.Rotator(), baseTrans.Rotator()).Quaternion());
		}
	}
}

void UPAnimFragment_Crouch::StartCrouch(bool bNewCrouch)
{
	if (!Data)
	{
		return;
	}

	bBlending = true;
	BlendTimeRate = OffsetBlendPct == 0.f ? 1.f : FMath::Max(OffsetBlendPct, 0.1f);
	OffsetBlendPct = 0.0f;

	InitialAlpha = Alpha;
	TargetAlpha = bNewCrouch ? 1.f : 0.f;
}

void UPAnimFragment_Crouch::UpdateCrouchAlpha(float DeltaTime)
{
	if (Data && bBlending)
	{
		if (OffsetBlendPct < 1.0f)
		{
			OffsetBlendPct = FMath::Min(OffsetBlendPct + (DeltaTime / Data->BlendTime) / BlendTimeRate, 1.0f);
			if (Data->BlendCurve)
			{
				Alpha = FMath::Lerp(InitialAlpha, TargetAlpha, Data->BlendCurve->GetFloatValue(OffsetBlendPct));
			}
			else
			{
				Alpha = FMath::InterpEaseInOut(InitialAlpha, TargetAlpha, OffsetBlendPct, 2.0f);
			}
		}
		else
		{
			Alpha = TargetAlpha;
			OffsetBlendPct = 0.f;
			bBlending = false;
		}
	}
}


UAnimSequence* UPAnimFragment_Crouch::GetBasePose(UAnimSequence* anim) const
{
	if (!Data || !anim)
	{
		return nullptr;
	}

	if (anim->AdditiveAnimType != AAT_None)
	{
		return anim->RefPoseSeq;
	}
	return Data->Pose_Base;
}
