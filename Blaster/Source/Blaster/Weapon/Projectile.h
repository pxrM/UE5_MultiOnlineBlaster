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


public:	


private:
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent; //����ʵ���ӵ���������䵯�����˶��������

	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer; //���ڴ����͹���������Ч����

	class UParticleSystemComponent* TracerComponent; //����ϵͳ��ص�����ࡣ������Ա����ڽ� "UParticleSystem" ������������Ч���ӵ���Ϸ�еĽ�ɫ���������������

};
