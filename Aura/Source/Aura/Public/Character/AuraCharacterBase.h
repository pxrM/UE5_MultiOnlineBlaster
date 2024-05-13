// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/*
 * 主角和小怪基类
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UAttributeSet* GetAttributeSet() const {return AttributeSet;}

	
protected:
	virtual void BeginPlay() override;

	
protected:
	// 角色武器mesh
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	/*
	 * GAS：
	 * 小怪的直接放在pawn上（怪物死亡后pawn消失，gas组件也会跟随pawn消失）
	 * 玩家的放在playerstate（不会跟随pawn消失而消失，可以复用）
	 */ 

	// 负责处理技能系统中涉及到的所有交互。任意Actor，只要它想要使用技能GameplayAbilities，拥有着属性Attributes，或者接收效果GameplayEffects，都必须附着一个ASC组件。
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	// 用于定义和跟踪角色的各种属性，如生命值、法力值、攻击力、防御力等。
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
