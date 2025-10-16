// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"


/*
 * 拾取器
 */
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
	/**
	 * 进入重叠，在 Actor（或 Actor 中的组件）与一个球形碰撞体积重叠时被调用。控制PickupWidget的显隐
	 * @param OverlappedComponent 指向正在重叠的组件的指针
	 * @param OtherActor 指向与此 Actor 重叠、在世界中存在的 Actor 的指针
	 * @param OtherComp 指向与此 Actor 重叠的另一组件的指针
	 * @param OtherBodyIndex 发生重叠的另一组件的索引
	 * @param bFromSweep 如果由移动/扫描操作引起重叠，则为 true；如果由物理引擎直接计算引起重叠，则为 false。
	 * @param SweepResult 描述重叠发生的详细信息，包括碰撞的位置、法向量等。
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f; //旋转速度
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent; //场景展示特效
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupDesEffect; //销毁时产生的特效

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();
};
