// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
	friend class ABlasterCharacter;	//是角色类可以访问武器类


private:
	class ABlasterCharacter* Character;
	//当前装备的武器
	UPROPERTY(Replicated)
		class AWeapon* EquippedWeapon;


public:
	/// <summary>
	/// 用于获取需要进行网络同步的属性列表
	/// </summary>
	/// <param name="OutLifetimeProps">需要进行网络同步的属性列表</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// <summary>
	/// 将武器装备到当前角色身上。只在服务器调用
	/// </summary>
	/// <param name="WeaponToEquip"></param>
	void EquipWeapon(AWeapon* WeaponToEquip);


};
