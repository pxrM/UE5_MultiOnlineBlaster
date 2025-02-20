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
	virtual void Die(const FVector& InDeathImpulse) override;
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
	virtual void ShowMagicCircle_Implementation(UMaterialInstance* DecalMaterial) override;
	virtual void HideMagicCircle_Implementation() override;
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;
	/* end IPlayerInterface */

	virtual void OnRep_Stunned() override;
	virtual void OnRep_Burned() override;

	// 加载本地保存的角色数据
	void LoadProgress();

private:
	// 初始化角色gas组件的相关信息
	virtual void InitAbilityActorInfo() override;

	// rpc广播角色升级效果
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComp;

	// 角色死亡后持续时间，该时间内用于表现角色死亡
	UPROPERTY(EditDefaultsOnly)
	float DeathTime = 5.f;

	// 声明一个计时器，用于角色死亡后一定时间处理后续逻辑
	FTimerHandle DeathTimer;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoomComp;
	
};
