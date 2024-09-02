// P


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,
                                                                const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// 监听标签的添加和删除
	AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
		WaitCooldownChange, &UWaitCooldownChange::CooldownTagChanged);

	// 当一个持续时间的游戏效果被添加到对象时将会触发的事件
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy(); // 标记一个对象为“准备销毁”。这意味着对象已经完成了它的工作，并且在下一个垃圾回收周期中会被销毁。
	MarkAsGarbage(); // 将对象标记为垃圾对象，以便在下一个垃圾回收过程中被清理。
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied,
                                              FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 获取设置到自身的所有标签
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	// 获取ge设置给actor的所有标签
	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	// GE是否应用了这个冷却标签
	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		// CooldownTag.GetSingleTagContainer()  返回只包含这个标签的GameplayTagContainer容器的引用。
		// FGameplayEffectQuery  用于查询游戏效果的对象。它定义了查询条件，并可以用来在游戏效果系统中查找符合这些条件的效果。
		// MakeQuery_MatchAnyOwningTags  创建一个效果查询，如果给定的标签和ActiveGameplayEffect的所属标签之间有任何共同的标签，该查询将进行匹配。
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
			CooldownTag.GetSingleTagContainer());
		// 获取查询到的所有包含冷却标签ge的剩余时间
		const TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// 避免由于某种原因，同时有多个冷却标签（比如没有及时清理），所以获取最高冷却时间
			float HighestTime = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if(TimesRemaining[i] > HighestTime)
				{
					HighestTime = TimesRemaining[i];
				}
			}
			// 广播冷却时间
			CooldownStart.Broadcast(HighestTime);
		}
	}
}
