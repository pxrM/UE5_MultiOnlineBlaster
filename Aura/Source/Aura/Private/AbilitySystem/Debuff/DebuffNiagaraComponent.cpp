// P


#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	// 设置自动激活为false
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
	if (UAbilitySystemComponent* Asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		Asc->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	else
	{
		if (CombatInterface)
		{
			// AddWeakLambda 这种绑定方式的主要好处是，当绑定的对象被销毁时，委托不会保持对象的引用，从而避免悬空指针问题和内存泄漏。
			CombatInterface->GetOnAscRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InAsc)
			{
				InAsc->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
					this, &UDebuffNiagaraComponent::DebuffTagChanged);
			});
		}
	}

	if(CombatInterface)
	{
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if(NewCount > 0)
	{
		Activate(); // 绑定的debuff标签大于0则激活特效
	}
	else
	{
		Deactivate(); // 关闭特效
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate(); // 关闭特效
}
