// P

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAuraAIController;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface, public IHighlightInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();

	// 将在Pawn被控制器（PlayerController和AIController）控制时触发回调
	virtual void PossessedBy(AController* NewController) override;

	/* start IHighlightInterface */
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	/* end IHighlightInterface */

	/* start IEnemyInterface */
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() override;
	/* end IEnemyInterface */

	/* start ICombatInterface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& InDeathImpulse) override;
	/*end ICombatInterface*/

	// 受击反应标签的监听委托函数
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// 眩晕标签改变
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;

	// 设置等级
	FORCEINLINE void SetLevel(const int32 InLevel) { Level = InLevel; }

protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	virtual void InitializeDefaultAttributes() const override;

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnLoot();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Default")
	int32 Level = 1;

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

	// 死亡后的存在时间
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float LifeSpan = 5.f;

	// 攻击目标
	UPROPERTY(BlueprintReadWrite, Category="Combat")
	TObjectPtr<AActor> CombatTarget;
};
