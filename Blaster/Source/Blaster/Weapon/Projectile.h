// Fill out your copyright notice in the Description page of Project Settings.
/*
	�䵯����
*/

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
	virtual void Destroyed() override; //�������л����ϵ���

	virtual void CollideManifestation(); //��ײ��ı���


protected:
	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //����ʵ���ӵ���������䵯�����˶��������


private:
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //���ڴ����͹���������Ч���࣬�ӵ�����ʱ��
	UPROPERTY()
		class UParticleSystemComponent* TracerComponent; //����ϵͳ��ص�����ࡣ������Ա����ڽ� "UParticleSystem" ������������Ч���ӵ���Ϸ�еĽ�ɫ���������������

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles; //ײ��ʱ��������Ч

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;  //ײ��ʱ��������Ч


protected:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		float DamageVal = 20.f; //�˺�


protected:
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
