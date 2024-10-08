// P

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;


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
	virtual void AddToXP_Implementation(const int32 InXP) override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 FindLevelForXP_Implementation(const int32 InXP) const override;
	virtual void LevelUp_Implementation() override;
	virtual int32 GetAttributePointReward_Implementation(const int32 Level) const override;
	virtual int32 GetSpellPointReward_Implementation(const int32 Level) const override;
	virtual void AddToAttributePoint_Implementation(const int32 InAttributePoint) override;
	virtual void AddToSpellPoint_Implementation(const int32 InSpellPoint) override;
	virtual void AddToPlayerLevel_Implementation(const int32 InPlayerLevel) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoint_Implementation() const override;
	/* end IPlayerInterface */

private:
	virtual void InitAbilityActorInfo() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComp;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoomComp;
	
};
