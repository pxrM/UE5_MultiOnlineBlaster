// P


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "Actor/AuraProjectileActor.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/* 在激活游戏能力时执行自定义逻辑 */
	UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);

	const bool bIsServer = HasAuthority(&ActivationInfo);
	if(!bIsServer) return;

	/*
	 * 使用 SpawnActorDeferred 可以在 actor 创建后立即获得其引用，此时还没有调用 BeginPlay 或完成初始化。因此，可以在生成后对 actor 进行一些额外的设置，再调用 FinishSpawningActor 完成生成过程。
	 *	ProjectileClass：生成的class类型
	 *	SpawnTransform：生成位置和旋转信息
	 *	GetOwningActorFromActorInfo()：拥有此能力的 actor，通常也是这个能力的执行者
	 *	Cast<APawn>(GetOwningActorFromActorInfo())：煽动者
	 *	ESpawnActorCollisionHandlingMethod::AlwaysSpawn：确定生成 actor 时的碰撞处理方式，这里选择 AlwaysSpawn 表示无论是否碰撞都生成 actor
	 */
	if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
	{
		const FVector SocketTransform = CombatInterface->GetCombatSocketLocation();
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketTransform);
		AActor* AbilityOwningActor = GetOwningActorFromActorInfo();
		AAuraProjectileActor* ProjectileActor = GetWorld()->SpawnActorDeferred<AAuraProjectileActor>(
			ProjectileClass,
			SpawnTransform,
			AbilityOwningActor,
			Cast<APawn>(AbilityOwningActor),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		// TODO: 给子弹一个造成伤害的 GameplayEffectSpec

		ProjectileActor->FinishSpawning(SpawnTransform);
	}
}
