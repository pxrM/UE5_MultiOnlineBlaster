// Fill out your copyright notice in the Description page of Project Settings.
/*
	�������һ����Χ�ڵĽ�ɫ����˺������Ը��˺�����˥��
*/

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();


protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	//��β��Ч�ӳ����ټ�ʱ����ɻص�
	void TrailDestroyTimerFinished();


private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* RocketMash;
	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMoveCmp;


protected:
	//��β��Ч����
	//UNiagaraSystem �����ڴ����͹��� Niagara ϵͳ���ࡣNiagara ��Ue��һ��ǿ�����Чϵͳ�����ڴ����߶ȿɶ��Ƶ����Ӻ���Ч��
	//UNiagaraSystem ������һϵ������ģ��Ͳ��������ڶ�����Ч����Ϊ����ۺͽ�����
	//UNiagaraSystem ���Ա����ڴ����͹���һ������ UNiagaraComponent ʵ�������ڳ����н���ʵ�����Ͳ��š�
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//��һ�� Unreal Engine �е�����࣬�����ڳ����з��úͿ��� Niagara ϵͳ��
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	//��β��Ч�ӳ����ټ�ʱ��
	FTimerHandle TrailDestroyTimer;
	//�ӳ�����ʱ��
	UPROPERTY(EditAnywhere)
		float TrailDestroyTime = 3.f;

	UPROPERTY(EditAnywhere)
		USoundCue* ProjectileLoop;
	UPROPERTY()
		UAudioComponent* ProjectileLoopComp;
	UPROPERTY(EditAnywhere)
		USoundAttenuation* LoopingSoundAttenuation; //����˥��

};
