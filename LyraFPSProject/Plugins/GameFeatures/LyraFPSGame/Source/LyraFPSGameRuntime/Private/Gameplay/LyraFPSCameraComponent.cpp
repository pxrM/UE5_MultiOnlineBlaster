// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/LyraFPSCameraComponent.h"

#include "LyraFPSFunctionLibrary.h"
#include "Gameplay/LyraFPSCameraMode.h"
#include "Gameplay/Cosmetic/SkeletalMeshComponent_FPS.h"

void ULyraFPSCameraModeStack::BlendFirstPersonParameters(float& OutFirstPersonFOV, float& OutFirstPersonScale, bool& bOutUseFirstPersonParameters) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	const auto GetFirstPersonParameters = [](const ULyraCameraMode* CameraMode, float& OutFOV, float& OutScale, bool& bOutUse)
	{
		if (const ULyraFPSCameraMode* FPSCameraMode = Cast<ULyraFPSCameraMode>(CameraMode))
		{
			OutFOV = FPSCameraMode->FirstPersonFieldOfView;
			OutScale = FPSCameraMode->FirstPersonScale;
			bOutUse = true;
			return;
		}

		OutFOV = CameraMode->GetCameraModeView().FieldOfView;
		OutScale = 1.0f;
		bOutUse = false;
	};

	GetFirstPersonParameters(CameraModeStack.Last(), OutFirstPersonFOV, OutFirstPersonScale, bOutUseFirstPersonParameters);
	for (int32 StackIndex = StackSize - 2; StackIndex >= 0; --StackIndex)
	{
		float OtherFOV = OutFirstPersonFOV;
		float OtherScale = OutFirstPersonScale;
		bool bOtherUseFirstPersonParameters = false;
		const ULyraCameraMode* CameraMode = CameraModeStack[StackIndex];
		GetFirstPersonParameters(CameraMode, OtherFOV, OtherScale, bOtherUseFirstPersonParameters);

		const float BlendWeight = CameraMode->GetBlendWeight();
		OutFirstPersonFOV = FMath::Lerp(OutFirstPersonFOV, OtherFOV, BlendWeight);
		OutFirstPersonScale = FMath::Lerp(OutFirstPersonScale, OtherScale, BlendWeight);
		bOutUseFirstPersonParameters = bOutUseFirstPersonParameters || bOtherUseFirstPersonParameters;
	}
}

ULyraCameraMode* ULyraFPSCameraModeStack::GetTopCameraMode() const
{
	return CameraModeStack.IsEmpty() ? nullptr : CameraModeStack[0];
}

ULyraFPSCameraComponent::ULyraFPSCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnableFirstPersonFieldOfView=true;
	bEnableFirstPersonScale=true;
	FirstPersonFieldOfView=97.f;
	FirstPersonScale=.5f;
}

void ULyraFPSCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!Cast<ULyraFPSCameraModeStack>(CameraModeStack))
	{
		CameraModeStack = NewObject<ULyraFPSCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void ULyraFPSCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);

	if (const ULyraFPSCameraModeStack* FPSCameraModeStack = Cast<ULyraFPSCameraModeStack>(CameraModeStack))
	{
		bool bUseFirstPersonParameters = DesiredView.bUseFirstPersonParameters;
		FPSCameraModeStack->BlendFirstPersonParameters(
			DesiredView.FirstPersonFOV,
			DesiredView.FirstPersonScale,
			bUseFirstPersonParameters);
		DesiredView.bUseFirstPersonParameters = bUseFirstPersonParameters;
	}
	
	if (auto FPSCameraMode=Cast<ULyraFPSCameraMode>( GetCurCameraMode()))
	{
		
		//查询
		float shakeRatio=0.f;
		if (!headCameraShadeRatioStack.IsEmpty())
		{
			shakeRatio=headCameraShadeRatioStack.Top().ShakeRatio;
		}
		else
		{
			shakeRatio=FPSCameraMode->GetHeadCameraShakeAdditiveRatio();
		}
		
		// 更新相机晃动位置
		if (shakeRatio>0.f)
		{
			if (USkeletalMeshComponent* MeshComp= ULyraFPSFunctionLibrary::GetFPSMesh(GetTargetActor()))
			{
				// FTransform HeadTrans= MeshComp->GetBoneTransform("Head",ERelativeTransformSpace::RTS_Component);
				FTransform HeadTrans= MeshComp->GetBoneTransform(FPSCameraMode->CameraShakeBoneName,FPSCameraMode->TransformSpace);
				if (InitCameraTransform.Equals(FTransform::Identity))
				{
					InitCameraTransform=GetCameraTransform( FPSCameraMode->InitSequence);
				}
				else
				{
					HeadTrans.SetLocation(HeadTrans.GetLocation()-InitCameraTransform.GetLocation());
					FRotator Rot=HeadTrans.GetRotation().Rotator();
					Rot.Yaw-=InitCameraTransform.Rotator().Yaw;
					
					float temp=Rot.Roll;
					Rot.Roll=Rot.Yaw;
					Rot.Yaw=temp;
					
					// DesiredView.Location += HeadTrans.GetLocation()* shakeRatio;
					DesiredView.Rotation += Rot * shakeRatio;
				}
			}
		}
	}
}

ULyraCameraMode* ULyraFPSCameraComponent::GetCurCameraMode()
{
	if (const ULyraFPSCameraModeStack* FPSCameraModeStack = Cast<ULyraFPSCameraModeStack>(CameraModeStack))
	{
		return FPSCameraModeStack->GetTopCameraMode();
	}

	return CameraModeStack->GetCurCameraMode();
}


FTransform ULyraFPSCameraComponent::GetCameraTransform(UAnimSequence*  InitSequence) const
{
	if (USkeletalMeshComponent_FPS* MeshComp= ULyraFPSFunctionLibrary::GetFPSMesh(GetTargetActor()))
	{
		if (UAnimSequence* anim=InitSequence)
		{
			auto boneIndex= FSkeletonPoseBoneIndex(MeshComp->GetBoneIndex("Head"));
			const FAnimExtractContext AnimExtractContext=FAnimExtractContext();
			FTransform OutAtom;
			anim->GetBoneTransform(OutAtom,boneIndex,AnimExtractContext,false);
			return OutAtom;
		}
	}
	return FTransform::Identity;
}

void ULyraFPSCameraComponent::AddCameraShakeRatio(FString ID, float ShakeRatio)
{
	headCameraShadeRatioStack.Push(FCameraShakeRatioPair(ID,ShakeRatio) );
}

void ULyraFPSCameraComponent::RemoveCameraShakeRatio(FString ID)
{
	headCameraShadeRatioStack.Remove(FCameraShakeRatioPair(ID));
}
