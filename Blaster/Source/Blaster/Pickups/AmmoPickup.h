// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 弹药
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
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
	/// <summary>
	/// 拾取的弹药数量
	/// </summary>
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;
	/// <summary>
	/// 拾取的武器类型
	/// </summary>
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

};
