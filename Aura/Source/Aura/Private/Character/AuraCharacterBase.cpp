// P


#include "Character/AuraCharacterBase.h"

#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false); // 设置胶囊体的OverlapEvent为false，防止和mesh上的发生冲突从而触发两次Overlap
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	if(MontageTag.MatchesTagExact(FAuraGameplayTags::Get().CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandTipSocketName);
	}
	if(MontageTag.MatchesTagExact(FAuraGameplayTags::Get().CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandTipSocketName);
	}
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Die()
{
	// 将武器从其当前父组件中分离
	//	FDetachmentTransformRules: 定义了如何进行分离的规则。
	//	 KeepWorld 表示在分离时保持在世界空间中的位置和旋转，而不是相对于其父组件。true 表示除了位置和旋转外，也保留组件的缩放信息
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	// 广播到client
	MulticastHandleDie();
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

void AAuraCharacterBase::MulticastHandleDie_Implementation()
{
	Weapon->SetSimulatePhysics(true); // 开启武器的物理模拟
	Weapon->SetEnableGravity(true); // 开启重力，使其可以自由掉落
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); // 设置碰撞为仅物理模拟，不触发碰撞逻辑

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 设置角色网格对于世界静态物理对象碰撞为阻塞(block)，使绝色不会穿过静态物理

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 将角色胶囊体组件的碰撞设置为不发生碰撞，这样角色在死亡后就不会再与其他物体碰撞

	DissolveMaterial();

	bDead = true;
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
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


