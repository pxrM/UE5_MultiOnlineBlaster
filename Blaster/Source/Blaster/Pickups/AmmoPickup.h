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
	/// ʰȡ���ӵ�����
	/// </summary>
	UPROPERTY(EditAnywhere)
		int32 AmmoAmount = 30;
	/// <summary>
	/// ʰȡ����
	/// </summary>
	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

};
