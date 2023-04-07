// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	friend class ABlasterCharacter;	//�ǽ�ɫ����Է���������

private:
	class ABlasterCharacter* Character;
	/// <summary>
	/// ��ǰ��ɫװ��������
	/// </summary>
	class AWeapon* EquippedWeapon;

public:
	/// <summary>
	/// ������װ������ǰ��ɫ����
	/// </summary>
	/// <param name="WeaponToEquip"></param>
	void EquipWeapon(AWeapon* WeaponToEquip);

		
};
