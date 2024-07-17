// P


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Actor/AuraProjectileActor.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/* 在激活游戏能力时执行自定义逻辑 */
	UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);
}


void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	/*
	 * 使用 SpawnActorDeferred 可以在 actor 创建后立即获得其引用，此时还没有调用 BeginPlay 或完成初始化。
	 * 因此，可以在生成后对 actor 进行一些额外的设置，再调用 FinishSpawningActor 完成生成过程。
	 *	ProjectileClass：生成的class类型
	 *	SpawnTransform：生成位置和旋转信息
	 *	GetOwningActorFromActorInfo()：拥有此能力的 actor，通常也是这个能力的执行者
	 *	Cast<APawn>(GetOwningActorFromActorInfo())：煽动者
	 *	ESpawnActorCollisionHandlingMethod::AlwaysSpawn：确定生成 actor 时的碰撞处理方式，这里选择 AlwaysSpawn 表示无论是否碰撞都生成 actor
	 */
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
	{
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo());
		FRotator Rotator = (ProjectileTargetLocation - SocketLocation).Rotation();
		// Rotator.Pitch = 0.f;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotator.Quaternion());

		AActor* AbilityOwningActor = GetOwningActorFromActorInfo();
		AAuraProjectileActor* ProjectileActor = GetWorld()->SpawnActorDeferred<AAuraProjectileActor>(
			ProjectileClass,
			SpawnTransform,
			AbilityOwningActor,
			Cast<APawn>(AbilityOwningActor),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		// 给子弹一个造成伤害的 GameplayEffectSpec(游戏效果规格用于描述应用到角色或者其他游戏实体上的具体游戏效果，例如造成伤害、治疗、添加状态效果等。)
		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(ProjectileActor);
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(ProjectileActor);
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

		for (auto& Pair : DamageTypes)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("FireBolt Damage: %f"), ScaledDamage));
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			// 将一个标签分配给一个由调用者指定的效果强度（Magnitude）
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
		
		ProjectileActor->DamageEffectSpecHandle = SpecHandle;

		ProjectileActor->FinishSpawning(SpawnTransform);
	}
}
