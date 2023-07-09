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
	virtual void Destroyed() override; //�������л����ϵ���

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	virtual void CollideManifestation(); //��ײ��ı���

	void SpawnTrailSystem(); //������β��Ч

	void StartDestroyTimer(); //�����ӳ����ټ�ʱ��
	void TrailDestroyTimerFinished(); //��β��Ч�ӳ����ټ�ʱ����ɻص�

	void ExplodeDamage();


protected:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		float DamageVal = 20.f; //�˺�

	//��β��Ч����
	//UNiagaraSystem �����ڴ����͹��� Niagara ϵͳ���ࡣNiagara ��Ue��һ��ǿ�����Чϵͳ�����ڴ����߶ȿɶ��Ƶ����Ӻ���Ч��
	//UNiagaraSystem ������һϵ������ģ��Ͳ��������ڶ�����Ч����Ϊ����ۺͽ�����
	//UNiagaraSystem ���Ա����ڴ����͹���һ������ UNiagaraComponent ʵ�������ڳ����н���ʵ�����Ͳ��š�
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//��һ�� Unreal Engine �е�����࣬�����ڳ����з��úͿ��� Niagara ϵͳ��
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //����ʵ���ӵ���������䵯�����˶��������

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMash;

	UPROPERTY(EditAnywhere)
		float DamageInnerRadius = 200.f; //��Χ�˺��ڰ뾶
	UPROPERTY(EditAnywhere)
		float DamageOuterRadius = 500.f; //��Χ�˺���뾶


private:
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //���ڴ����͹���������Ч���࣬�ӵ�����ʱ��
	UPROPERTY()
		class UParticleSystemComponent* TracerComponent; //����ϵͳ��ص�����ࡣ������Ա����ڽ� "UParticleSystem" ������������Ч���ӵ���Ϸ�еĽ�ɫ���������������

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles; //ײ��ʱ��������Ч

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;  //ײ��ʱ��������Ч

	//��β��Ч�ӳ����ټ�ʱ��
	FTimerHandle TrailDestroyTimer;
	//�ӳ�����ʱ��
	UPROPERTY(EditAnywhere)
		float TrailDestroyTime = 3.f;


};
