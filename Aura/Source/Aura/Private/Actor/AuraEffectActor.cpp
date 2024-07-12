// P


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemy) return;

	check(GameplayEffectClass);

	// IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(Target);
	// UAuraAbilitySystemComponent* TargetAsc = AscInterface->GetAbilitySystemComponent();
	
	UAbilitySystemComponent* TargetAsc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(TargetAsc == nullptr) return;
	
	// 创建EffectContext实例，并返回这个实例的指针，随后使用结构FGameplayEffectContextHandle来包裹它
	// FGameplayEffectContext是UE中用于表示游戏效果上下文的结构体。它包含了应用游戏效果时所需的关键信息，如来源、目标、施法者等。
	// GE 就是配置文件，而配置文件携带的信息一般来说是固定的；
	FGameplayEffectContextHandle EffectContextHandle = TargetAsc->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);

	// 创建FGameplayEffectSpec实例，并返回这个实例的指针，随后使用结构FGameplayEffectSpecHandle来包裹它
	// FGameplayEffectSpec用于辅助UGameplayEffect的结构，用于保存实例化之后的Effect与刚创建出来的Context，同时还会记录对属性的修改、计算Modifier的持续时间、捕获定义等等。
	// 给目标施加 GE，除了GE本身配置还需要一些其它参数（比如：GE等级、什么时候施加GE、谁施加的GE等、GE的目标是谁等）。因此施加GE需要包装一层
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetAsc->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);

	// 将游戏效果规范应用到自身。ActiveGameplayEffectHandle用来存储应用的游戏效果的句柄。这个句柄可以用于后续的操作，比如删除或修改已应用的游戏效果。
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetAsc->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// 先通过EffectSpecHandle.Data.Get()获取到游戏效果FGameplayEffectSpec对象
	// 然后通过Def.Get()获取到UGameplayEffect对象
	// 最后检查了该游戏效果定义的持续时间策略是否为EGameplayEffectDurationType::Infinite（永久的）
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if(bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetAsc);
	}

	// 如果设置了应用时删除，除了Infinite都会自动删除
	if(bDestroyOnEffectApplication && !bIsInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemy) return;
	
	if(InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemy) return;
	
	if(InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}

	if(InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetAsc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if(!IsValid(TargetAsc)) return;

		TArray<FActiveGameplayEffectHandle> HandleToRemove;
		for(const TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair :ActiveEffectHandles)
		{
			if(TargetAsc == HandlePair.Value)
			{
				TargetAsc->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandleToRemove.Add(HandlePair.Key);
			}
		}
		for(FActiveGameplayEffectHandle& Handle : HandleToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}

