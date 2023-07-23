// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
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
	EFT_HitScan UMETA(DisplayName = "单射线射击"),
	EFT_Projectile UMETA(DisplayName = "子弹射击"),
	EFT_Shotgun UMETA(DisplayName = "多射线射击"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX"),
};

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


private:
	UPROPERTY()
		class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
		class ABlasterPlayerController* BlasterOwnerController;

	/// <summary>
	/// 武器网格
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* WeaponMesh;
	/// <summary>
	/// 角色触发区域
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;
	/// <summary>
	/// 武器状态
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;
	/// <summary>
	/// 拾取 umg
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;
	/// <summary>
	/// 开火动画资源
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;
	/// <summary>
	/// 开火时产生的子弹壳类
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		TSubclassOf<class ACasing> CasingClass;
	/// <summary>
	/// 弹夹容量中剩余子弹数量
	/// </summary>
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AmmoNum, Category = "Weapon Properties")
		int32 AmmoNum;
	/// <summary>
	/// 弹夹容量
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		int32 MagCapacity;
	/// <summary>
	/// 武器类型
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		EWeaponType WeaponType;
	/// <summary>
	/// 武器开火子弹类型
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		EFireType FireType;


public:
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter; // 十字瞄准贴图  中
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsLeft;			// 十字瞄准贴图  左
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsRight;		// 十字瞄准贴图  右
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsTop;			// 十字瞄准贴图  上
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsBottom;		// 十字瞄准贴图  下

	/// <summary>
	/// 瞄准缩放
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Zoom)
		float ZoomedFOV = 30.f;
	/// <summary>
	/// 缩放速度
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Zoom)
		float ZoomInterpSpeed = 20.f;

	/// <summary>
	/// 是否自动开火
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
		bool bAutomatic = true;
	/// <summary>
	/// 开火间隔时间
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = 0.15f;

	/// <summary>
	/// 装备武器音效
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Combat)
		class USoundCue* EquipSouund;

	/* 分散子弹 */
	/// <summary>
	/// 分散球体和枪口的距离
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	/// <summary>
	/// 分散球体半径
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	/// <summary>
	/// 是否使用分散子弹
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseSactter = false;

	/// <summary>
	/// 玩家淘汰时是否销毁武器，一般只对玩家的默认武器生效
	/// </summary>
	bool bDestroyWeapon = false;


protected:
	/// <summary>
	/// 进入重叠，在 Actor（或 Actor 中的组件）与一个球形碰撞体积重叠时被调用。控制PickupWidget的显隐
	/// </summary>
	/// <param name="OverlappedComponent">指向正在重叠的组件的指针</param>
	/// <param name="OtherActor">指向与此 Actor 重叠、在世界中存在的 Actor 的指针</param>
	/// <param name="OtherComp">指向与此 Actor 重叠的另一组件的指针</param>
	/// <param name="OtherBodyIndex">发生重叠的另一组件的索引</param>
	/// <param name="bFromSweep">如果由移动/扫描操作引起重叠，则为 true；如果由物理引擎直接计算引起重叠，则为 false。</param>
	/// <param name="SweepResult">描述重叠发生的详细信息，包括碰撞的位置、法向量等。</param>
	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);
	/// <summary>
	/// 退出重叠
	/// </summary>
	UFUNCTION()
		virtual void OnSphereEndOerlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

	/* 设置武器状态 */
	virtual void OnSetWeaponState();
	virtual void OnEquippedState();
	virtual void OnEquippedSecondaryState();
	virtual void OnDroppedState();


public:
	void ShowPickupWidget(bool bShow);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	void Dropped(); //掉落
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);
	void EnableCustomDepth(bool bEnable); //是否开启RnderCustomDepth，以显示轮廓
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

	/// <summary>
	/// 获取一个扩散后的目标方向
	/// </summary>
	/// <param name="TraceStart">起始点，废弃，改为由该函数自己计算</param>
	/// <param name="HitTarget">射线目标点</param>
	/// <returns></returns>
	FVector TraceEndWithScatter(const FVector& HitTarget);


private:
	UFUNCTION()
		void OnRep_WeaponState();
	UFUNCTION()
		void OnRep_AmmoNum();
	void SpeedRound();

};
