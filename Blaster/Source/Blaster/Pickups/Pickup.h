// Fill out your copyright notice in the Description page of Project Settings.
/*
	拾取器
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
	/// 进入重叠，在 Actor（或 Actor 中的组件）与一个球形碰撞体积重叠时被调用。控制PickupWidget的显隐
	/// </summary>
	/// <param name="OverlappedComponent">指向正在重叠的组件的指针</param>
	/// <param name="OtherActor">指向与此 Actor 重叠、在世界中存在的 Actor 的指针</param>
	/// <param name="OtherComp">指向与此 Actor 重叠的另一组件的指针</param>
	/// <param name="OtherBodyIndex">发生重叠的另一组件的索引</param>
	/// <param name="bFromSweep">如果由移动/扫描操作引起重叠，则为 true；如果由物理引擎直接计算引起重叠，则为 false。</param>
	/// <param name="SweepResult">描述重叠发生的详细信息，包括碰撞的位置、法向量等。</param>
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
