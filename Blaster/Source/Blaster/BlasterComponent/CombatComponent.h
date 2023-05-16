// Fill out your copyright notice in the Description page of Project Settings.
/*
	角色战斗组件 管理武器
*/

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
	class ABlasterPlayerController* Controller;
	class ABlasterHUD* HUD;

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

	float CrosshairVelocityFactor; //十字准线的缩放，射击游戏中十字准线会根据角色的位置移动稍微张开
	float CrosshairInAirFactor;

	FVector HitTarget;	//射击目标位置


	float DefultFOV;	//没瞄准时的默认视野
	float CurrentFOV;	//当前视野
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;  //瞄准时的放大视野
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;  //瞄准时的视野缩放速度


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

	/*  FVector_NetQuantize 是ue中用于网络传输的结构体，用于压缩和优化 FVector 的数据传输。
		该结构体可以将 FVector 的值在网络传输时进行压缩，使数据大小更小，减少网络负载和传输延迟。
		FVector_NetQuantize 支持每个分量最多使用 20 位二进制数，在精度和实时性之间做了一个平衡。*/
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	//标记为 NetMulticast 和 Reliable。这意味着该函数将在各个客户端上进行调用，并且该函数的执行结果将从客户端同步到服务器和其他客户端。
	//在多人游戏中，此函数通常用于向所有客户端广播某些操作，例如在所有客户端上生成爆炸效果。
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	//射线检测，用于检测玩家准心位置所对应的世界空间位置和方向
	void TraceUnderCroshairs(FHitResult& TraceHitResult);

	//设置hud十字准线
	void SetHUDCrosshairs(float DeltaTime);

private:
	//瞄准时处理视野缩放的函数
	void InterpFOV(float DeltaTime);

};
