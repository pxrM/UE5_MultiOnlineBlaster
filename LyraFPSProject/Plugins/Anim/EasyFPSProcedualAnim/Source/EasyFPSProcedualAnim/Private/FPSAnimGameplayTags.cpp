// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAnimGameplayTags.h"

namespace FPSAnimGameplayTags
{
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment, "ProceduralAnim.Fragment", "Fragment mark");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Jump, "ProceduralAnim.Fragment.Type.Jump", "Jump InAir Land");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Crouch, "ProceduralAnim.Fragment.Type.Crouch", "Crouch");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Walk, "ProceduralAnim.Fragment.Type.Walk", "Walk");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Wiggle, "ProceduralAnim.Fragment.Type.Wiggle", "Wiggle");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Target, "ProceduralAnim.Fragment.Target", "作用对象");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Target_Camera, "ProceduralAnim.Fragment.Target.Camera", "相机");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_Target_ViewModel, "ProceduralAnim.Fragment.Target.ViewModel", "视觉模型");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event, "ProceduralAnim.Event", "事件");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_JumpStart, "ProceduralAnim.Event.Jump.JumpStart", "开始跳跃事件");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_InAir, "ProceduralAnim.Event.Jump.InAir", "跳跃_空中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Landed, "ProceduralAnim.Event.Jump.Landed", "跳跃_落地");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_PlayMontage, "ProceduralAnim.Event.PlayMontage", "播放蒙太奇");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Signal, "ProceduralAnim.Signal", "信号");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Signal_Jump, "ProceduralAnim.Signal.Jump", "跳跃");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Signal_Land, "ProceduralAnim.Signal.Land", "跳跃");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Signal_Crouch, "ProceduralAnim.Signal.Crouch", "跳跃");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Signal_UnCrouch, "ProceduralAnim.Signal.UnCrouch", "跳跃");
	
	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}