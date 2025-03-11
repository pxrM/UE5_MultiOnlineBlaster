// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 治疗buff
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;


private:
	/// <summary>
	/// 治疗量
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;
	
	/// <summary>
	/// 治疗时间
	/// </summary>
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

};
