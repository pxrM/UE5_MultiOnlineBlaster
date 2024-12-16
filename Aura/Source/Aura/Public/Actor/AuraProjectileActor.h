// P

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "AuraProjectileActor.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraProjectileActor();

	
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	void OnHit();

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereCmp;

	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;
	
	bool bHit = false;
	
	// 碰撞后的特效
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	// 碰撞后的音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;

	// 子弹飞行时的持续音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundCmp;
	

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementCmp;

	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;
	
	/*
	 * FGameplayEffectSpecHandle是一个句柄，它封装了指向FGameplayEffectSpec(描述了一个GameplayEffect将如何应用以及其具体参数)结构的指针。 
	 * meta=(ExposeOnSpawn = true)
	 * 该元数据属性表示在生成这个对象时，可以在构造函数参数中设置这个属性的值。这样可以在创建对象时直接初始化这个属性，而不需要在创建后单独设置。
	 *
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;
	*/
	
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;
};
 