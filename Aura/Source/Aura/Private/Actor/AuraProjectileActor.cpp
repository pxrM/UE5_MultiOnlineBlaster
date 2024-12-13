// P


#include "Actor/AuraProjectileActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"

AAuraProjectileActor::AAuraProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereCmp = CreateDefaultSubobject<USphereComponent>("SphereCmp");
	SetRootComponent(SphereCmp);
	SphereCmp->SetCollisionObjectType(ECC_Projectile);
	SphereCmp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCmp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCmp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereCmp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereCmp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovementCmp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementCmp");
	ProjectileMovementCmp->InitialSpeed = 550.f;
	ProjectileMovementCmp->MaxSpeed = 550.f;
	ProjectileMovementCmp->ProjectileGravityScale = 0.f;
}

void AAuraProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan); // 设置该 Actor 的生命周期时间。当生命周期到期时，Actor 会自动销毁。

	SphereCmp->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectileActor::OnSphereOverlap);

	LoopingSoundCmp = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AAuraProjectileActor::Destroyed()
{
	// 证明当前没有触发Overlap事件，在销毁前播放击中特效
	if (!bHit && !HasAuthority())
	{
		OnHit();
	}
	Super::Destroyed();
}

void AAuraProjectileActor::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopingSoundCmp) LoopingSoundCmp->Stop();
	bHit = true;
}

void AAuraProjectileActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                           const FHitResult& SweepResult)
{
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemCmp->GetAvatarActor();
	// 发射者和碰撞的对象
	if (GetInstigator() == OtherActor) return;
	if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;

	if (!bHit) OnHit();
	
	if (HasAuthority())
	{
		// Server
		// 应用伤害
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// 碰撞时获取技能的朝向
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			DamageEffectParams.TargetAbilitySystemCmp = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		Destroy();
	}
	else
	{
		// Client
		// 防止客户端在被销毁之前发生重叠（网络问题）
		bHit = true;
	}
}
