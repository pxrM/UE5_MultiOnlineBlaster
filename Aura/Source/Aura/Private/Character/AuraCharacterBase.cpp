// P


#include "Character/AuraCharacterBase.h"

#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	BurnNiagaraComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("BurnDebuffComp");
	BurnNiagaraComponent->SetupAttachment(GetRootComponent());
	BurnNiagaraComponent->DebuffTag = FAuraGameplayTags::Get().DeBuff_Burn;
	StunNiagaraComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("StunDebuffComp");
	StunNiagaraComponent->SetupAttachment(GetRootComponent());
	StunNiagaraComponent->DebuffTag = FAuraGameplayTags::Get().DeBuff_Stun;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false); // 设置胶囊体的OverlapEvent为false，防止和mesh上的发生冲突从而触发两次Overlap
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EffectAttachCmp = CreateDefaultSubobject<USceneComponent>("EffectAttachPoint");
	EffectAttachCmp->SetupAttachment(GetRootComponent());
	HaloOfProtectionNiagaraCmp = CreateDefaultSubobject<UPassiveNiagaraComponent>("HaloOfProtectionNiagaraComponent");
	HaloOfProtectionNiagaraCmp->SetupAttachment(EffectAttachCmp);
	LifeNiagaraCmp = CreateDefaultSubobject<UPassiveNiagaraComponent>("LifeNiagaraComponent");
	LifeNiagaraCmp->SetupAttachment(EffectAttachCmp);
	ManaNiagaraCmp = CreateDefaultSubobject<UPassiveNiagaraComponent>("ManaNiagaraComponent");
	ManaNiagaraCmp->SetupAttachment(EffectAttachCmp);
	
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

void AAuraCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 防止特效跟随人物旋转，每一帧更新修改旋转为默认
	EffectAttachCmp->SetWorldRotation(FRotator::ZeroRotator);
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBurned);
	DOREPLIFETIME(AAuraCharacterBase, IsBeingShocked);
}

float AAuraCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageSignature.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag)
{
	if(SocketTag.MatchesTagExact(FAuraGameplayTags::Get().CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandTipSocketName);
	}
	if(SocketTag.MatchesTagExact(FAuraGameplayTags::Get().CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandTipSocketName);
	}
	if(SocketTag.MatchesTagExact(FAuraGameplayTags::Get().CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Die(const FVector& InDeathImpulse)
{
	// 将武器从其当前父组件中分离
	//	FDetachmentTransformRules: 定义了如何进行分离的规则。
	//	 KeepWorld 表示在分离时保持在世界空间中的位置和旋转，而不是相对于其父组件。true 表示除了位置和旋转外，也保留组件的缩放信息
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	// 广播到client
	MulticastHandleDie(InDeathImpulse);
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontages_Implementation() const
{
	return AttackMontages;
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for(FTaggedMontage TaggedMontage : AttackMontages)
	{
		if(TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

int32 AAuraCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void AAuraCharacterBase::IncrementalMinionCount_Implementation(const int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClassType AAuraCharacterBase::GetCharacterType_Implementation()
{
	return CharacterClassType;
}

FOnASCRegistered& AAuraCharacterBase::GetOnAscRegisteredDelegate()
{
	return OnAscRegisteredDelegate;
}

FOnDeath& AAuraCharacterBase::GetOnDeathDelegate()
{
	return OnDeathDelegate;
}

USkeletalMeshComponent* AAuraCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

void AAuraCharacterBase::SetIsBeingShocked_Implementation(const bool bInShock)
{
	IsBeingShocked = bInShock;
}

bool AAuraCharacterBase::IsBeingShocked_Implementation() const
{
	return IsBeingShocked;
}

FOnDamageSignature& AAuraCharacterBase::GetOnDamageSignature()
{
	return OnDamageSignature;
}

void AAuraCharacterBase::MulticastHandleDie_Implementation(const FVector& InDeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());
	
	Weapon->SetSimulatePhysics(true); // 开启武器的物理模拟
	Weapon->SetEnableGravity(true); // 开启重力，使其可以自由掉落
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); // 设置碰撞为仅物理模拟，不触发碰撞逻辑
	Weapon->AddImpulse(InDeathImpulse * 0.1f, NAME_None, true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 设置角色网格对于世界静态物理对象碰撞为阻塞(block)，使绝色不会穿过静态物理
	GetMesh()->AddImpulse(InDeathImpulse, NAME_None, true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 将角色胶囊体组件的碰撞设置为不发生碰撞，这样角色在死亡后就不会再与其他物体碰撞

	DissolveMaterial();
	bDead = true;
	BurnNiagaraComponent->Deactivate();
	StunNiagaraComponent->Deactivate();
	OnDeathDelegate.Broadcast(this);
}

void AAuraCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f: BaseWalkSpeed;
}

void AAuraCharacterBase::OnRep_Stunned()
{
	
}

void AAuraCharacterBase::OnRep_Burned()
{
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, const float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	if(!HasAuthority()) return;

	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->AddCharacterAbilities(StartupAbilities);
	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AAuraCharacterBase::DissolveMaterial()
{
	// 需要基于实例材质制作一个动态材质实例（这样才能动态修改），然后设置给角色
	if(IsValid(AvatarDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInt = UMaterialInstanceDynamic::Create(AvatarDissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInt);
		StartAvatarDissolveTimeline(DynamicMatInt);
	}
	if(IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInt = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInt);
		StartWeaponDissolveTimeline(DynamicMatInt);
	}
}


