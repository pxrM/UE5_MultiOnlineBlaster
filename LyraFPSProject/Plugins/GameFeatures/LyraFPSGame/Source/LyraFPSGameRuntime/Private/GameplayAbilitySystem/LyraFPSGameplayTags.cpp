// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/LyraFPSGameplayTags.h"

#include "GameplayTagContainer.h"

namespace LyraFPSGameplayTags
{
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Sprint, "InputTag.Ability.Sprint", "Sprint input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ablity_Sprint, "Ability.Type.Action.Sprint", "Sprint");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AblityCue_Death_FPS, "GameplayCue.Character.Death.FPS", "FPSDeath");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Sprinting, "Status.Sprinting", "Target is sprinting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_SprintInputHeld, "Status.Sprinting.InputHeld", "Sprint input is currently held.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_SprintFireBlocked, "Status.Sprinting.FireBlocked", "Sprint is currently blocking weapon fire.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Sprint_LocalAnim, "Status.Sprinting.LocalAnim", "跟随FireBlocked，但用于本地动画判断");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_MovementDisabled, "Status.Movement.Disable", "禁止移动");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_KnockedDown, "Status.Movement.KnockedDown", "被击倒");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Stunned, "Status.Movement.Stunned", "昏迷");
	
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(PlayerStart_AI, "PlayerStart.AI", "AI生成位置");
	
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
