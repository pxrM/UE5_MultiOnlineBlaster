// P


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	// 这个委托用于在Effect应用到自身角色时触发相应的逻辑 (OnGameplayEffectAppliedDelegateToSelf - Server)
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);

	//const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("Tag:%s"),*GameplayTags.Attributes_Secondary_Armor.ToString()));
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// GEngine->AddOnScreenDebugMessage(1, 8.f, FColor::Blue, FString("Effect Applied"));

	// 获取所有标签并填充到容器中
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	
	// for(const FGameplayTag& Tag : TagContainer)
	// {
	// 	// const FString Msg = FString::Printf(TEXT("GE Tag: %s"), *Tag.ToString());
	// 	// GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, Msg);
	// }
	
	EffectAssetTags.Broadcast(TagContainer);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for(const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// 如果转换UAuraGameplayAbility成功，则添加动态标签到该能力的标签集合中
		if(const UAuraGameplayAbility* Ability = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// DynamicAbilityTags允许为特定的能力实例添加动态标签。这些标签可以用于在运行时调整和管理能力的行为，比如条件激活、冷却时间、能力互斥等
			AbilitySpec.DynamicAbilityTags.AddTag(Ability->StartupInputTag);
			// 只应用给角色不激活
			GiveAbility(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	// 输入的标签是否有效
	if(!InputTag.IsValid()) return;

	// 遍历可激活的能力列表
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 检查该能力是否具有指定的动态标签
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 告知GameplayAbility，此技能被触发按下事件
			// 会触发技能上面的回调
			// virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
			AbilitySpecInputPressed(AbilitySpec);
			// 如果能力实例尚未激活，则尝试激活
			if(!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid()) return;

	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 告知技能按键事件被抬起
			// 会触发技能上面的回调
			// virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}
