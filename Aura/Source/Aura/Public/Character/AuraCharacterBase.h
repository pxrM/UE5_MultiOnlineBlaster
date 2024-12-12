// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UDebuffNiagaraComponent;
class UNiagaraSystem;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;

/*
 * 主角和小怪基类
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UAttributeSet* GetAttributeSet() const {return AttributeSet;}

	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual void Die() override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() const override;
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
	virtual int32 GetMinionCount_Implementation() override;
	virtual void IncrementalMinionCount_Implementation(const int32 Amount) override;
	virtual ECharacterClassType GetCharacterType_Implementation() override;
	virtual FOnASCRegistered& GetOnAscRegisteredDelegate() override;
	virtual FOnDeath& GetOnDeathDelegate() override;

	// 将死亡同步到server和client
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDie();

	
protected:
	virtual void BeginPlay() override;

	// 初始化gas组件上的actor信息
	virtual void InitAbilityActorInfo();

	// 添加ge到自身
	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, const float Level) const;

	// 初始化属性值
	virtual void InitializeDefaultAttributes() const;

	// 向角色添加能力
	void AddCharacterAbilities();

	// 实例溶解材质并替换
	void DissolveMaterial();
	// 溶解需要一个时间过程，在蓝图里面实现时间轴，增加一个蓝图实现的函数，因为不确定有几个材质需要修改，所以设置了一个数组，可能角色和武器两个都需要修改
	UFUNCTION(BlueprintImplementableEvent)
	void StartAvatarDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debuff")
	TObjectPtr<UDebuffNiagaraComponent> DebuffNiagaraComponent;
	// 角色武器mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	// 武器上的施法插槽名
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandTipSocketName;
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandTipSocketName;
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName;
	// 角色溶解材质实例
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> AvatarDissolveMaterialInstance;
	// 武器溶解材质实例
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
	// 击中流血效果
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UNiagaraSystem* BloodEffect;
	// 死亡音效
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;
	// 是否死亡
	bool bDead = false;
	// 小兵数量（召唤出来的小怪、宠物等）
	int32 MinionCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Default")
	ECharacterClassType CharacterClassType = ECharacterClassType::Warrior;
	
	/*
	 * GAS：
	 * 挂载位置：
	 *	小怪的直接放在pawn上（怪物死亡后pawn消失，gas组件也会跟随pawn消失）。此时gas的OwnerActor和AvatarActor都是这个pawn
	 *	玩家的放在playerstate（不会跟随pawn消失而消失，可以复用）。此时gas的OwnerActor是这个playerstate，而AvatarActor才是这个pawn
	 *
	 * 网络复制模式（SetReplicationMode）：
	 *	Minimal 	Multiplayer,AI-Controlled 		Effect不复制，Cues和Tags会被复制到所有client
	 *	Mixed		Multiplayer,Player-Controlled	Effect只复制到自己的client，Cues和Tags会被复制到所有client
	 *	Full		SinglePlayer(单人游戏)			GAS数据都会被复制到所有client
	 *	对于Mixed（混合复制）模式:OwnerActor的Owner必须是Controller。
	 *	依附pawn，这个在possession()中自动设置。依附PlayerState的Owner被自动设置为Controller。
	 *	因此，如果你的OwnerActor不是PlayerState，并且使用混合复制模式，你必须在OwnerActor上调用SetOwner()来将其owner设置为Controller。
	 *
	 * 初始化AbilityActorInfo（UAbilitySystemComponent::InitAbilityActorInfo）时机：
	 *	Player-Controlled Character
	 *		依附在Pawn上
	 *			server：PossessedBy
	 *			client：AcknowledgePossession
	 *		依附在PlayerState
	 *			server：PossessedBy
	 *			client：OnRep_PlayerState
	 *	AI-Controlled Character
	 *		依附在Pawn上
	 *			server & client：BeginPlay
	 *
	 */

	// 负责处理技能系统中涉及到的所有交互。任意Actor，只要它想要使用技能GameplayAbilities，拥有着属性Attributes，或者接收效果GameplayEffects，都必须附着一个ASC组件。
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	// 用于定义和跟踪角色的各种属性，如生命值、法力值、攻击力、防御力等。
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TObjectPtr<UAttributeSet> AttributeSet;
	// 用来初始化角色上的Primary相关属性值
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	// 次要属性
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;
	// 必要属性
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;


private:
	// 游戏开始赋予角色的能力
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	// 游戏开始赋予角色的被动能力
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;
	// 受击蒙太奇
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;


public:
	// 设置多个基础攻击动画
	UPROPERTY(EditAnywhere,Category="Combat")
	TArray<FTaggedMontage> AttackMontages;

	FOnASCRegistered OnAscRegisteredDelegate;
	FOnDeath OnDeathDelegate;
};
