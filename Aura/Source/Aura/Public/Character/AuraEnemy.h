// P

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();
	
	/* start IEnemyInterface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/* end IEnemyInterface */

	/* start ICombatInterface */
	virtual int32 GetPlayerLevel() override;
	/*end ICombatInterface*/


protected:
	virtual void BeginPlay() override;
	
	virtual void InitAbilityActorInfo() override;
	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Default")
	int32 Level = 1;
	
};
