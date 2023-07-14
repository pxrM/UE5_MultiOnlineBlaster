// Fill out your copyright notice in the Description page of Project Settings.
/*
	ʰȡ��
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	/// <summary>
	/// �����ص����� Actor���� Actor �е��������һ��������ײ����ص�ʱ�����á�����PickupWidget������
	/// </summary>
	/// <param name="OverlappedComponent">ָ�������ص��������ָ��</param>
	/// <param name="OtherActor">ָ����� Actor �ص����������д��ڵ� Actor ��ָ��</param>
	/// <param name="OtherComp">ָ����� Actor �ص�����һ�����ָ��</param>
	/// <param name="OtherBodyIndex">�����ص�����һ���������</param>
	/// <param name="bFromSweep">������ƶ�/ɨ����������ص�����Ϊ true���������������ֱ�Ӽ��������ص�����Ϊ false��</param>
	/// <param name="SweepResult">�����ص���������ϸ��Ϣ��������ײ��λ�á��������ȡ�</param>
	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);


public:	
	UPROPERTY(EditAnywhere)
		class USphereComponent* OverlapSphere;
	UPROPERTY(EditAnywhere)
		class USoundCue* PickupSound;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* PickupMesh;
};
