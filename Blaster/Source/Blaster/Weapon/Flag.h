// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 旗帜，继承武器方便使用武器的拾取功能
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

protected:
	virtual void BeginPlay() override;
	virtual void OnEquippedState() override;
	virtual void OnDroppedState() override;

public:
	virtual void Dropped() override;
	void ResetFlag();

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* FlagMesh;

	/// <summary>
	/// 旗帜的初始位置，方便回归
	/// </summary>
	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
	
};
