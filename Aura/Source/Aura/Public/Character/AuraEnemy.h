// P

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAuraAIController;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();
	// 将在Pawn被控制器（PlayerController和AIController）控制时触发回调
	virtual void PossessedBy(AController* NewController) override;
	
	/* start IEnemyInterface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/* end IEnemyInterface */

	/* start ICombatInterface */
	virtual int32 GetPlayerLevel() override;
	/*end ICombatInterface*/

	// 受击反应标签的监听委托函数
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	virtual void Die() override;


protected:
	virtual void BeginPlay() override;
	
	virtual void InitAbilityActorInfo() override;

	virtual void InitializeDefaultAttributes() const override;
	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Default")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Default")
	ECharacterClassType CharacterClass = ECharacterClassType::Warrior;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, Category="AI") 
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;


public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	// 击中反应
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	bool bHitReacting = false;

	// 行走速度
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	float BaseWalkSpeed = 250.f;

	// 死亡后的存在时间
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float LifeSpan = 5.f;
	
};
