// P

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UPassiveNiagaraComponent();

protected:
	virtual void BeginPlay() override;

	void OnPassiveSpellTag(const FGameplayTag& AbilityTag, const bool bActivate);

public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;
};
