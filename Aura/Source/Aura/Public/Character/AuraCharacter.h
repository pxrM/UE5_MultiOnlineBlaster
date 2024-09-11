// P

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	// 当角色（Pawn）被控制器（Controller）所控制时，此函数将被调用
	virtual void PossessedBy(AController* NewController) override;
	// 在玩家状态（PlayerState）在网络中被复制时被调用
	virtual void OnRep_PlayerState() override;

	/* start ICombatInterface */
	virtual int32 GetPlayerLevel_Implementation() override;
	/* end ICombatInterface */

	/* start IPlayerInterface */
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	/* end IPlayerInterface */


private:
	virtual void InitAbilityActorInfo() override;
};
