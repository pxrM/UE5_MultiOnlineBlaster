// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000.f

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
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAiming; //是否正在瞄准

	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed; //原始速度

	UPROPERTY(EditAnywhere)
		float AimWalkSpeed; //瞄准时速度

	bool bFireBtnPressed; //是否按下开火键

	FVector HitTarget; //攻击坐标


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


protected:
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	UFUNCTION()
		void FireBtnPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
		void ServerFire();

	//标记为 NetMulticast 和 Reliable。这意味着该函数将在各个客户端上进行调用，并且该函数的执行结果将从客户端同步到服务器和其他客户端。
	//在多人游戏中，此函数通常用于向所有客户端广播某些操作，例如在所有客户端上生成爆炸效果。
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire();

	void TraceUnderCroshairs(FHitResult& TraceHitResult);

};
