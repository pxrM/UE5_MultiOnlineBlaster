// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

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
	UPROPERTY(EditAnywhere)
		float BaseSpeedBuff = 1600.f; //速度
	UPROPERTY(EditAnywhere)
		float CrouchSpeedBuff = 850.f; //蹲伏速度
	UPROPERTY(EditAnywhere)
		float SpeedBuffTime = 30.f; //速度buff增益时间
};
