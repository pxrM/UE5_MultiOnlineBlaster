// P


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAuraAbilitySystemComponent* AuraAsc = Cast<UAuraAbilitySystemComponent>(
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		AuraAsc->ActivePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveSpellTag);
	}
	else
	{
		if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
		{
			CombatInterface->GetOnAscRegisteredDelegate().AddWeakLambda(this,[this](UAbilitySystemComponent* InAsc)
			{
				if(UAuraAbilitySystemComponent* Asc = Cast<UAuraAbilitySystemComponent>(InAsc))
				{
					Asc->ActivePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveSpellTag);
				}
			});
		}
	}
}

void UPassiveNiagaraComponent::OnPassiveSpellTag(const FGameplayTag& AbilityTag, const bool bActivate)
{
	if(AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if(bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
}
