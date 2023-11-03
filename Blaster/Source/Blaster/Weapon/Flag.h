// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * ���ģ��̳���������ʹ��������ʰȡ����
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

public:
	virtual void Dropped() override;

protected:
	virtual void OnEquippedState() override;
	virtual void OnDroppedState() override;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* FlagMesh;

	FVector InitialLocation;

public:
	FORCEINLINE FVector GetInitialLocation() const { return InitialLocation; }
	
};
