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


class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterHUD;
class AWeapon;
class AProjectile;


/*
* 角色战斗组件
* 
* 主要功能：
* 1. 武器系统管理（装备、切换、掉落）
* 2. 战斗状态控制（瞄准、射击、换弹）
* 3. 弹药系统管理（弹药携带、消耗、补充）
* 4. HUD界面更新（准心、弹药显示）
* 5. 投掷物系统（手榴弹）
* 6. 多人同步处理
*/

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;	//角色类可以访问武器组件类

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


public:
	/// <summary>
	/// 将武器装备到当前角色身上。只在服务器调用
	/// </summary>
	/// <param name="WeaponToEquip"></param>
	void EquipWeapon(AWeapon* WeaponToEquip);
	/// <summary>
	/// 交换武器
	/// </summary>
	void SwapWeapons();

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
	/// 交换武器蒙太奇播放完成
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void FinishSwapMontage();
	/// <summary>
	/// 交换武器蒙太奇播放到附加武器阶段
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();

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

	/// <summary>
	/// 投掷手榴弹蒙太奇动画结束
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	/// <summary>
	/// 投掷手榴弹蒙太奇动画播放至扔出手榴弹时刻
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	/// <summary>
	/// 生成手榴弹
	/// </summary>
	/// <param name="Target">FVector_NetQuantize提供在网络传输中压缩和序列化三维向量功能，提高网络传输性能。</param>
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	/// <summary>
	/// 拾取子弹
	/// </summary>
	/// <param name="WeaponType">子弹的武器类型</param>
	/// <param name="AmmoAmount">子弹数量</param>
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	/// <summary>
	/// 是否可以交换武器
	/// </summary>
	bool IsShouldSwapWeapons();
	/// <summary>
	/// 获取手榴弹数量
	/// </summary>
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	

protected:
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UFUNCTION()
	void OnRep_Aiming();

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	/**
	 * 使当前装备的武器掉落
	 */
	void DropEquippedWeapon();
	/**
	 * 附加actor到角色的右手
	 * @param ActorToAttach 要附加的actor
	 */
	void AttachActorToRightHand(AActor* ActorToAttach);
	/**
	 * 附加actor到角色的左手
	 * @param ActorToAttach 要附加的actor
	 */
	void AttachActorToLeftHand(AActor* ActorToAttach);
	/**
	 * 附加actor到角色的背包
	 * @param ActorToAttach 要附加的actor
	 */
	void AttachActorToBackpack(AActor* ActorToAttach);
	/** 
	 * 附加旗帜到角色的左手
	 * @param Flag 要附加的旗帜
	 */
	void AttachFlagToLeftHand(AWeapon* Flag);
	/**
	 * 更新当前装备武器的携带弹药量及hud
	 */
	void UpdateCarriedAmmo();
	/**
	 * 播放装备武器音效
	 */
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	/**
	 * 更换武器空弹夹
	 */
	void ReloadEmptyWeapon();
	
	/**
	 * 核心射击控制函数
	 * 功能：
	 * 1. 检查是否可以开火（通过 CanFire() 判断）
	 * 2. 根据当前装备武器的类型执行不同的射击逻辑
	 * 3. 控制射击节奏（通过 FireTimer）
	 * 4. 更新准心视觉反馈
	 * 射击类型：
	 * - EFT_Projectile：发射物理弹道的投射物（如火箭弹）
	 * - EFT_HitScan：即时射线检测（如步枪）
	 * - EFT_Shotgun：霰弹枪特殊处理
	 * 调用时机：
	 * - 由 FireBtnPressed() 在玩家按下开火键时触发
	 * - 在自动武器模式下由 FireTimer 定时触发
	 * 注意：
	 * - 开火后会暂时禁用开火能力（bCanFire = false）
	 * - 通过 StartFireTimer() 控制下一次开火时机
	 * - 会增加准心扩散效果（CrosshairShootingFactor）
	 */
	void Fire();
	/**
	 * 本地-开火
	 * @param TraceHitTarget 射击目标位置
	 */
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	/**
	 * 本地霰弹枪开火
	 */
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	/**
	 * 本地-发射投射物武器
	 */
	void FireProjectileWeapon();
	/**
	 * 本地-发射即时射线武器
	 */
	void FireHitScanWeapon();
	/**
	 * 本地-发射霰弹枪武器
	 */
	void FireShotgunWeapon();

	/**
	 * 服务器执行开火的RPC函数
	 * 网络角色：
	 * - 在客户端调用，在服务器端执行
	 * - 使用 Server 标记表示这是一个客户端到服务器的RPC
	 * - Reliable 确保消息一定会到达服务器
	 * - WithValidation 提供额外的验证检查
	 * 
	 * 参数说明：
	 * FVector_NetQuantize 是ue中用于网络传输的结构体，用于压缩和优化 FVector 的数据传输。
	 * 该结构体可以将 FVector 的值在网络传输时进行压缩，使数据大小更小，减少网络负载和传输延迟。
	 * FVector_NetQuantize 支持每个分量最多使用 20 位二进制数，在精度和实时性之间做了一个平衡。
	 * @param TraceHitTarget 射击目标位置，使用 FVector_NetQuantize 进行网络优化
	 *        - FVector_NetQuantize 是专门用于网络传输的压缩向量
	 *        - 每个分量使用20位二进制表示，在精度和带宽之间取得平衡
	 * @param FireDelay 射击延迟时间
	 *        - 用于控制射击的节奏
	 *        - 可以用于实现射击频率限制
	 * 
	 * 工作流程：
	 * 1. 客户端检测到射击输入
	 * 2. 客户端计算射击目标位置
	 * 3. 调用此函数将射击请求发送给服务器
	 * 4. 服务器验证并执行射击
	 * 5. 服务器通过 MulticastFire 将结果广播给所有客户端
	 * 
	 * 注意事项：
	 * - 作为可靠RPC，会增加网络负载，但确保射击不会丢失
	 * - WithValidation 要求实现一个配套的验证函数
	 * - 通常配合 MulticastFire 使用，形成完整的网络同步
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);
	/**
	 * 多播-开火
	 * 标记为 NetMulticast 和 Reliable。这意味着该函数将在各个客户端上进行调用，并且该函数的执行结果将从客户端同步到服务器和其他客户端。
	 * 在多人游戏中，此函数通常用于向所有客户端广播某些操作，例如在所有客户端上生成爆炸效果。
	 * @param TraceHitTarget 射击目标位置
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	/**
	 * 服务器-霰弹枪开火
	 * @param TraceHitTargets 射击目标位置
	 * @param FireDelay 开火延迟
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	/**
	 * 多播-霰弹枪开火
	 * @param TraceHitTargets 射击目标位置
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	/**
	 * 服务器-重新加载弹夹
	 */
	UFUNCTION(Server, Reliable)
	void ServerReloadMag();
	/**
	 * 服务器和客户端调用-处理重新加载弹夹
	 */
	void HandleReloadMag();	//
	/**
	 * 更换弹匣后子弹的变化数量
	 */
	int32 AmountToReloadMag();

	/**
	 * 射线检测，用于检测玩家准心位置所对应的世界空间位置和方向
	 * @param TraceHitResult 射线检测结果
	 */
	void TraceUnderCroshairs(FHitResult& TraceHitResult);
	/**
	 * 设置hud十字准线
	 * @param DeltaTime 时间
	 */
	void SetHUDCrosshairs(float DeltaTime);

	/**	
	 * 投掷手榴弹
	 */
	void ThrowGrenade();
	/**
	 * 服务器-投掷手榴弹
	 */
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	/**
	 * 显示附加手榴弹
	 */
	void ShowAttachedGrenade(bool bShowGrenade);

	/**
	 * 装备主武器
	 */
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	/**
	 * 装备副武器
	 */
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);


private:
	/**
	 * 瞄准时处理视野缩放的函数
	 * @param DeltaTime 时间
	 */
	void InterpFOV(float DeltaTime);

	/**
	 * 启动自动开火计时器
	 */
	void StartFireTimer();
	/**
	 * 自动开火计时器回调
	 */
	void FireTimerFinished();

	/**
	 * 是否可开火
	 */
	bool CanFire();

	/**
	 * 初始化弹夹
	 */
	void InitializeCarriedAmmo();

	/**
	 * 更新弹夹的子弹数量
	 */
	void UpdateAmmoValues();
	/**
	 * 更新霰弹枪弹夹的子弹数量
	 */
	void UpdateShotgunAmmoValues();

	UFUNCTION()
	void OnRep_CurWeaponCarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_Grenades();
	void UpdateHUDGrenades();

	UFUNCTION()
	void OnRep_HoldingTheFlag();


private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()
	ABlasterPlayerController* Controller;
	UPROPERTY()
	ABlasterHUD* HUD;

	/**
	 * 当前装备的武器
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	/**
	 * 第二把武器
	 */
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	/**
	 * 是否正在瞄准
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;
	/**
	 * 是否按下瞄准键
	 */
	bool bAimBtnPressed = false;

	/**
	 * 原始速度
	 */
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	/**
	 * 瞄准时速度
	 */
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/**
	 * 是否按下开火键
	 */
	bool bFireBtnPressed; //是否按下开火键

	/**
	 * 射击目标位置
	 */
	FVector HitTarget;

	/*----十字准线 start----*/
	float CrosshairVelocityFactor; //十字准线的缩放，射击游戏中十字准线会根据角色的位置移动稍微张开
	float CrosshairInAirFactor;    //角色在空中时十字准线的缩放
	float CrosshairInAimFactor;    //角色在瞄准时十字准线的缩放
	float CrosshairShootingFactor; //角色在开火时十字准线的缩放
	FHUDPackage HUDPackage; //十字准线贴图结构体
	/*----十字准线 end----*/

	/*----瞄准视野 start----*/
	/**
	 * 没瞄准时的默认视野
	 */
	float DefultFOV;
	/**
	 * 当前视野
	 */
	float CurrentFOV;
	/**
	 * 瞄准时的放大视野
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	/**
	 * 瞄准时的视野缩放速度
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;
	/*----瞄准视野 end----*/

	/*----自动开火 start----*/
	/**
	 * 开火计时器
	 */
	FTimerHandle FireTimer;
	/**
	 * 是否可以开火
	 */
	bool bCanFire = true;
	/*----自动开火 end----*/

	/**
	 * 本地变量，用来处理高延迟问题，是否在更换弹夹中
	 */
	bool bLocallyReloading = false;

	/**
	 * 不同武器类型的携带弹药量
	 */
	TMap<EWeaponType, int32> CarriedAmmoMap;

	/**
	 * 当前武器类型的携带弹药量
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CurWeaponCarriedAmmo)
	int32 CurWeaponCarriedAmmo;
	
	/*----初始化携带弹药量 start----*/
	/**
	 * 用来初始化步枪武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingARAmmo = 30;
	/**
	 * 用来初始化火箭武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingRocketAmmo = 0;
	/**
	 * 用来初始化手枪武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingPistolAmmo = 0;
	/**
	 * 用来初始化冲锋枪武器的携带弹药量
	 */	
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingSMGAmmo = 0;
	/**
	 * 用来初始化霰弹枪武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingShotgunAmmo = 0;
	/**
	 * 用来初始化狙击步枪武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingSniperAmmo = 0;
	/**
	 * 用来初始化榴弹武器的携带弹药量
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 StartingGrenadeAmmo = 0;
	/*----初始化携带弹药量 end----*/
	
	/**
	 * 最大武器弹药携带量，这里适用于每种武器
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	int32 MaxCarriedAmmo = 500;

	/**
	 * 当前拥有的手榴弹数量
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;
	/**
	 * 最大拥有手榴弹数量
	 */
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades;

	/**
	 * 战斗状态
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	/**
	 * 旗帜
	 */
	UPROPERTY()
	AWeapon* TheFlag;
	/**
	 * 是否持有旗帜
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;


protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass; //投掷弹药类-手榴弹

};
