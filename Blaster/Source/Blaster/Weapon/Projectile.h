// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //����ʵ���ӵ���������䵯�����˶��������

	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //���ڴ����͹���������Ч���࣬�ӵ�����ʱ��

	class UParticleSystemComponent* TracerComponent; //����ϵͳ��ص�����ࡣ������Ա����ڽ� "UParticleSystem" ������������Ч���ӵ���Ϸ�еĽ�ɫ���������������

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles; //ײ��ʱ��������Ч

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;  //ײ��ʱ��������Ч


protected:
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
