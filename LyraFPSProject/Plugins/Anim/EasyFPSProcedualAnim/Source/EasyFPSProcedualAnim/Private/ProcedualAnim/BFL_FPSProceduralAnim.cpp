// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/BFL_FPSProceduralAnim.h"

#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

// #include "Editor/AnimationBlueprintLibrary/Public/AnimationBlueprintLibrary.h"

FTransform UBFL_FPSProceduralAnim::GetBoneTrans_Pose(UAnimSequence* Animation, FName BoneName,float Time)
{
	FTransform transform = FTransform::Identity;
	if (Animation && Animation->GetSkeleton())
	{
		// float length= anim->GetPlayLength();
		const USkeleton* Skeleton = Animation->GetSkeleton();
		const int32 boneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
		if (boneIndex == INDEX_NONE)
		{
			return transform;
		}
		FSkeletonPoseBoneIndex BoneIndex(boneIndex);
		FAnimExtractContext extractionContext(static_cast<double>(Time));
		Animation->GetBoneTransform(transform,BoneIndex,extractionContext,true);
		// UAnimationBlueprintLibrary::GetBonePoseForFrame();
		// UAnimationBlueprintLibrary::GetTimeAtFrame(Time,extractionContext);
		// UAnimationBlueprintLibrary::frame
		// UE_LOG(LogTemp,Warning,TEXT("GetBoneTrans:"));
	}
	return transform;
}
