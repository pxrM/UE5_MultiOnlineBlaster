// P

#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "AuraMeleeAttack.generated.h"

/**
 * 基于之前的伤害技能基类，创建一个近战技能基类
 */
UCLASS()
class AURA_API UAuraMeleeAttack : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
};
