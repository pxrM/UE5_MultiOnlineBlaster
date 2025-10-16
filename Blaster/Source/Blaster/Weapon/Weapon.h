// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial UMETA(DisplayName = "初始"),
	EWS_Equipped UMETA(DisplayName = "装备"),
	EWS_EquippedSecondary UMETA(DisplayName = "装备第二把武器"),
	EWS_Dropped UMETA(DisplayName = "掉落"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UENUM(BlueprintType)
enum class EFireType :uint8
{
	EFT_HitScan UMETA(DisplayName = "单射线射击（如步枪）"),
	EFT_Projectile UMETA(DisplayName = "子弹射击（如火箭筒）"),
	EFT_Shotgun UMETA(DisplayName = "多射线射击（如霰弹枪）"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX"),
};


class USphereComponent;
class UTexture2D;
class USoundCue;
class ABlasterCharacter;
class ABlasterPlayerController;
class UWidgetComponent;
class UAnimationAsset;
class ACasing;


/*
 * 武器类
 */
UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;


protected:
	virtual void BeginPlay() override;

	/**
	 * 重叠
	 * @param OverlappedComponent 指向正在重叠的组件的指针
	 * @param OtherActor 指向与此 Actor 重叠、在世界中存在的 Actor 的指针
	 * @param OtherComp 指向与此 Actor 重叠的另一组件的指针
	 * @param OtherBodyIndex 发生重叠的另一组件的索引
	 * @param bFromSweep 如果由移动/扫描操作引起重叠，则为 true；如果由物理引擎直接计算引起重叠，则为 false。
	 * @param SweepResult 描述重叠发生的详细信息，包括碰撞的位置、法向量等。
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	/**
	 * 退出重叠
	 */
	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/**
	 * 设置武器状态
	 */
	virtual void OnSetWeaponState();
	virtual void OnEquippedState();
	virtual void OnEquippedSecondaryState();
	virtual void OnDroppedState();

	/**
	 * 启用/禁用服务器倒带  当ping太高时，禁用服务器倒带
	 * @param bPingTooHigh 是否ping太高
	 */
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);


public:
	/**
	 * 设置显示拾取UI
	 * @param bShow 是否显示
	 */
	void ShowPickupWidget(bool bShow);
	/**
	 * 设置武器状态
	 * @param State 武器状态
	 */
	void SetWeaponState(EWeaponState State);
	/**
	 * 开火
	 * @param HitTarget 射线目标点
	 */
	virtual void Fire(const FVector& HitTarget);
	/**
	 * 设置武器掉落
	 */
	virtual void Dropped();
	/**
	 * 更新HUD的子弹数量
	 */
	void SetHUDAmmo();
	/**
	 * 添加子弹数量
	 * @param AmmoToAdd 添加的子弹数量
	 */
	void AddAmmo(int32 AmmoToAdd);
	/**
	 * 是否开启RnderCustomDepth，以显示轮廓
	 * @param bEnable 是否开启
	 */
	void EnableCustomDepth(const bool bEnable) const;
	/**
	 * 获取一个扩散后的目标方向
	 * @param TraceStart 起始点，废弃，改为由该函数自己计算
	 * @param HitTarget 射线目标点
	 * @returns 扩散后的目标方向
	 */
	FVector TraceEndWithScatter(const FVector& HitTarget);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsAmmoEmpty() const { return AmmoNum <= 0; }
	FORCEINLINE bool IsAmmoFull() const { return AmmoNum == MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE int32 GetAmmoNum() const { return AmmoNum; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE ETeam GetTeam() const { return Team; }


private:
	UFUNCTION()
	void OnRep_WeaponState();
	//UFUNCTION()
	//	void OnRep_AmmoNum();
	void SpeedRound();
	/**
	 * 服务器通知客户端更新子弹数量
	 * @param ServerAmmo 服务器子弹数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	/**
	 * 服务器通知客户端添加子弹数量
	 * @param AmmoToAdd 添加的子弹数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);


protected:
	UPROPERTY()
	ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController;

	/* 分散子弹 */
	/**
	 * 分散球体和枪口的距离
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;
	/**
	 * 分散球体半径
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	/**
	 * 武器伤害
	 */
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	/**
	 * 爆头伤害
	 */
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	/**
	 * 是否启用服务器倒带
	 */
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;



private:
	/**
	 * 武器网格
	 */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	/**
	 * 角色触发区域
	 */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;
	/**
	 * 武器状态
	 */
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	/**
	 * 拾取 umg
	 */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;
	/**
	 * 开火动画资源
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;
	/**
	 * 开火时产生的子弹壳类
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<ACasing> CasingClass;
	/**
	 * 弹夹容量中剩余子弹数量
	 */
	UPROPERTY(EditAnywhere, /*ReplicatedUsing = OnRep_AmmoNum,*/ Category = "Weapon Properties")
	int32 AmmoNum;
	/**
	 * 弹夹容量
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 MagCapacity;
	/**
	 * 武器类型
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;
	/**
	 * 武器开火子弹类型
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EFireType FireType;
	/**
	 * 客户端预测，需要存储最后的更新和一个请求服务器的消息序列号，
	 * 当服务器回消息时，检查序列号，并检查有多少未处理的请求，然后处理更改。
	 * 未处理的服务器同步过来Ammo的同步次数数量。
	 * 在每一轮SpeedRound中增加，在ClientUpdateAmmo中减少
	 */
	int32 SequenceAmmo = 0;
	/**
	 * 所属队伍
	 */
	UPROPERTY(EditAnywhere)
	ETeam Team;


public:
	/**
	 * 十字瞄准贴图
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;
	/**
	 * 十字瞄准贴图  左
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;
	/**
	 * 十字瞄准贴图  右
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;
	/**
	 * 十字瞄准贴图  上
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;
	/**
	 * 十字瞄准贴图  下
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/**
	 * 瞄准缩放
	 */
	UPROPERTY(EditAnywhere, Category = Zoom)
	float ZoomedFOV = 30.f;
	/**
	 * 缩放速度
	 */
	UPROPERTY(EditAnywhere, Category = Zoom)
	float ZoomInterpSpeed = 20.f;

	/**
	 * 是否自动开火
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	/**
	 * 开火间隔时间
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	/**
	 * 装备武器音效
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	USoundCue* EquipSouund;

	/**
	 * 玩家淘汰时是否销毁武器，一般只对玩家的默认武器生效
	 */
	bool bDestroyWeapon = false;

	/**
	 * 是否使用分散子弹
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseSactter = false;

};
