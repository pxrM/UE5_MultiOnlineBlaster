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
	/// ���ĵĳ�ʼλ�ã�����ع�
	/// </summary>
	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
	
};
