// Fill out your copyright notice in the Description page of Project Settings.
/*
	角色战斗组件 管理武器
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;	//是角色类可以访问武器类

public:
	// Sets default values for this component's properties
	UCombatComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/// <summary>
	/// 用于获取需要进行网络同步的属性列表
	/// </summary>
	/// <param name="OutLifetimeProps">需要进行网络同步的属性列表</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


private:
	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	UPROPERTY()
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

	FVector HitTarget;	//射击目标位置

	/*	 十字准线	*/
	float CrosshairVelocityFactor; //十字准线的缩放，射击游戏中十字准线会根据角色的位置移动稍微张开
	float CrosshairInAirFactor;    //角色在空中时十字准线的缩放
	float CrosshairInAimFactor;    //角色在瞄准时十字准线的缩放
	float CrosshairShootingFactor; //角色在开火时十字准线的缩放
	FHUDPackage HUDPackage; //十字准线贴图结构体

	/*	 瞄准视野	*/
	float DefultFOV;	//没瞄准时的默认视野
	float CurrentFOV;	//当前视野
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;  //瞄准时的放大视野
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;  //瞄准时的视野缩放速度

	/*	 自动开火	*/
	FTimerHandle FireTimer;	//开火计时器
	bool bCanFire = true; //是否可以开火

	TMap<EWeaponType, int32> CarriedAmmoMap;	//不同武器类型的携带弹药量
	UPROPERTY(ReplicatedUsing = OnRep_CurWeaponCarriedAmmo)
		int32 CurWeaponCarriedAmmo;  //携带弹药量（角色当前武器类型的弹药数量）
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingARAmmo = 30;	//用来初始化步枪武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingRocketAmmo = 0;	//用来初始化火箭武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingPistolAmmo = 0;	//用来初始化手枪武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingSMGAmmo = 0;	//用来初始化冲锋枪武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingShotgunAmmo = 0;	//用来初始化霰弹枪武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingSniperAmmo = 0;	//用来初始化狙击步枪武器的携带弹药量
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingGrenadeAmmo = 0;	//用来初始化榴弹武器的携带弹药量

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied; //战斗状态

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
		int32 Grenades = 4; //当前拥有的手榴弹数量
	UPROPERTY(EditAnywhere)
		int32 MaxGrenades; //最大拥有手榴弹数量


protected:
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectile> GrenadeClass; //投掷弹药类-手榴弹


public:
	/// <summary>
	/// 将武器装备到当前角色身上。只在服务器调用
	/// </summary>
	/// <param name="WeaponToEquip"></param>
	void EquipWeapon(AWeapon* WeaponToEquip);

	/// <summary>
	/// 重新加载弹夹，客户端发起
	/// </summary>
	void ReloadMag();

	/// <summary>
	/// 重新加载弹夹结束
	/// 在动画蒙太奇中添加动画事件调用Game/Blueprints/Character/Animation/ReloadMag
	/// </summary>
	UFUNCTION(BlueprintCallable)
		void FinishReloadMag();

	/// <summary>
	/// 开火
	/// </summary>
	/// <param name="bPressed"></param>
	UFUNCTION()
		void FireBtnPressed(bool bPressed);

	/// <summary>
	/// 霰弹枪更换弹夹时动画填装子弹的回调
	/// </summary>
	UFUNCTION(BlueprintCallable)
		void ShotgunShellReload();
	void AnimJumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
		void ThrowGrenadeFinished(); //投掷手榴弹蒙太奇动画结束
	UFUNCTION(BlueprintCallable)
		void LaunchGrenade(); //投掷手榴弹蒙太奇动画播放至扔出手榴弹时刻
	UFUNCTION(Server, Reliable)
		void ServerLaunchGrenade(const FVector_NetQuantize& Target);//生成手榴弹。FVector_NetQuantize提供在网络传输中压缩和序列化三维向量功能，提高网络传输性能。


protected:
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();
	void DropEquippedWeapon();  //使当前装备的武器掉落
	void AttachActorToRightHand(AActor* ActorToAttach);	//附加actor到角色的右手
	void AttachActorToLeftHand(AActor* ActorToAttach);	//附加actor到角色的左手
	void UpdateCarriedAmmo(); //更新武器的携带弹药量
	void PlayEquipWeaponSound(); //播放装备武器音效
	void ReloadEmptyWeapon(); //更换武器空弹夹

	void Fire();
	/*  FVector_NetQuantize 是ue中用于网络传输的结构体，用于压缩和优化 FVector 的数据传输。
		该结构体可以将 FVector 的值在网络传输时进行压缩，使数据大小更小，减少网络负载和传输延迟。
		FVector_NetQuantize 支持每个分量最多使用 20 位二进制数，在精度和实时性之间做了一个平衡。*/
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	//标记为 NetMulticast 和 Reliable。这意味着该函数将在各个客户端上进行调用，并且该函数的执行结果将从客户端同步到服务器和其他客户端。
	//在多人游戏中，此函数通常用于向所有客户端广播某些操作，例如在所有客户端上生成爆炸效果。
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
		void ServerReloadMag();	//服务器调用
	void HandleReloadMag();	//服务器和客户端调用
	//更换弹匣后子弹的变化数量
	int32 AmountToReloadMag();

	//射线检测，用于检测玩家准心位置所对应的世界空间位置和方向
	void TraceUnderCroshairs(FHitResult& TraceHitResult);
	//设置hud十字准线
	void SetHUDCrosshairs(float DeltaTime);

	//投掷手榴弹
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
		void ServerThrowGrenade();
	void ShowAttachedGrenade(bool bShowGrenade);


private:
	//瞄准时处理视野缩放的函数
	void InterpFOV(float DeltaTime);

	//启动自动开火计时器
	void StartFireTimer();
	//自动开火计时器回调
	void FireTimerFinished();

	//是否可开火
	bool CanFire();

	//初始化弹夹
	void InitializeCarriedAmmo();

	//更新弹夹的子弹数量
	void UpdateAmmoValues();
	//更新霰弹枪弹夹的子弹数量
	void UpdateShotgunAmmoValues();

	UFUNCTION()
		void OnRep_CurWeaponCarriedAmmo();

	UFUNCTION()
		void OnRep_CombatState();

	UFUNCTION()
		void OnRep_Grenades();
	void UpdateHUDGrenades();


public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }

};
