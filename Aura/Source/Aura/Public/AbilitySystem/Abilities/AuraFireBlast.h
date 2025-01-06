// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraFireBlast.generated.h"

/**
 * 火焰爆炸技能
 */
UCLASS()
class AURA_API UAuraFireBlast : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextDescription(int32 Level) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireBlast")
	int32 NumFireBalls;
};
