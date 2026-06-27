// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/Fragments/PAnimFragment_Wiggle.h"

#include "Animation/AnimSequence.h"
#include "FPSAnimGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProcedualAnim/BFL_FPSProceduralAnim.h"

namespace
{
	FTransform MakeWiggleDelta(const FTransform& BoneTrans, const FTransform& BaseTrans)
	{
		return FTransform(
			UKismetMathLibrary::NormalizedDeltaRotator(BoneTrans.Rotator(), BaseTrans.Rotator()),
			BoneTrans.GetLocation() - BaseTrans.GetLocation(),
			FVector::OneVector);
	}
}

UPAnimFragment_Wiggle::UPAnimFragment_Wiggle()
{
	Tag= FPSAnimGameplayTags::Fragment_Wiggle;
}

void UPAnimFragment_Wiggle::Initialize(UFPSProceduralAnimComp* InOwnerComp)
{
	Super::Initialize(InOwnerComp);
	
}

void UPAnimFragment_Wiggle::Tick(float DeltaTime)
{
	Offset = FVector::ZeroVector;
	Rot = FRotator::ZeroRotator;

	if (!Data)
	{
		Super::Tick(DeltaTime);
		return;
	}

	if (CurAnim)
	{
		if (CurAnimTime >= CurAnim->GetPlayLength())
		{
			CurAnim = nullptr;
			CurAnimTime = 0.f;
			CurBlendInTime = 0.f;
			bIsPlaying_CurAnim = false;
		}
		else
		{
			float BlendRate=1.f;
			if (Data->BlendInTime>0.f)
			{
				BlendRate=FMath::Min(1.f,CurBlendInTime/Data->BlendInTime);
			}
		
			FTransform curAnimTrans= GetAnimTrans(CurAnim,CurAnimTime);
			Offset+=curAnimTrans.GetLocation()*BlendRate;
			Rot+=curAnimTrans.Rotator()*BlendRate;
			CurAnimTime+=DeltaTime;
		
			CurBlendInTime+=DeltaTime;
		}
	}
	if (PreAnim)
	{
		if (Data->BlendOutTime > 0.f)
		{
			CurBlendOutTime+=DeltaTime;
			FTransform preAnimTrans = GetAnimTrans(PreAnim,PreAnimTime);
			float BlendRate=FMath::Clamp(1.f-CurBlendOutTime/Data->BlendOutTime, 0.f, 1.f);
			Offset+=preAnimTrans.GetLocation()*BlendRate;
			Rot+=preAnimTrans.Rotator()*BlendRate;
			PreAnimTime+=DeltaTime;
		
			if (BlendRate<=0.f)
			{
				PreAnim=nullptr;
				PreAnimTime=0.f;
				CurBlendOutTime=0.f;
				bIsPlaying_PreAnim = false;
			}
		}
		else
		{
			PreAnim=nullptr;
			PreAnimTime=0.f;
			CurBlendOutTime=0.f;
			bIsPlaying_PreAnim = false;
		}
	}
	Super::Tick(DeltaTime);
}

void UPAnimFragment_Wiggle::ReceiveSignal(const FGameplayTag& Signal)
{
	StartAnimation(Signal);
}

bool UPAnimFragment_Wiggle::SetData(UDataAsset_ProceduralAnim* InData)
{
	Data=Cast<UDataAsset_Anim_Wiggle>(InData);
	return Data?true:false;
}

void UPAnimFragment_Wiggle::StartAnimation(FGameplayTag tag)
{
	if (!Data)
	{
		return;
	}

	if (auto animPtr= Data->Anims.Find(tag))
	{
		if (auto newAnim= *animPtr)
		{
			if (CurAnim)
			{
				PreAnim=CurAnim;
				PreAnimTime=CurAnimTime;
				CurBlendOutTime=0.f;
				bIsPlaying_PreAnim = true;
			}
			CurAnim=newAnim;
			CurAnimTime=0.f;
			CurBlendInTime=0.f;
			bIsPlaying_CurAnim = true;
		}
	}
}

FTransform UPAnimFragment_Wiggle::GetAnimTrans(UAnimSequence* InAnim,float InTime) const
{
	if (!InAnim || !Data)
	{
		return FTransform::Identity;
	}

	const float SampleTime = FMath::Clamp(InTime, 0.f, InAnim->GetPlayLength());
	const FTransform BoneTrans = UBFL_FPSProceduralAnim::GetBoneTrans_Pose(InAnim, Data->BoneName, SampleTime);
	FTransform BaseTrans = FTransform::Identity;

	if (InAnim->AdditiveAnimType != AAT_None)
	{
		BaseTrans = UBFL_FPSProceduralAnim::GetBoneTrans_Pose(InAnim, Data->BoneName, 0.f);
	}
	else
	{
		UAnimSequence* BasePose = GetBasePose(InAnim);
		if (!BasePose)
		{
			return FTransform::Identity;
		}
		BaseTrans = UBFL_FPSProceduralAnim::GetBoneTrans_Pose(BasePose, Data->BoneName, SampleTime);
	}
	
	const FTransform Result = MakeWiggleDelta(BoneTrans, BaseTrans);
	return Result.ContainsNaN() ? FTransform::Identity : Result;
}

UAnimSequence* UPAnimFragment_Wiggle::GetBasePose(UAnimSequence* anim) const
{
	if (!Data || !anim || anim->AdditiveAnimType != AAT_None)
	{
		return nullptr;
	}
	return Data->DefaultBasePose;
}
