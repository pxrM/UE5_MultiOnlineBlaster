// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterCharacter.generated.h"


//退出当前游戏
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/// <summary>
	/// 用于获取需要进行网络同步的属性列表
	/// </summary>
	/// <param name="OutLifetimeProps">需要进行网络同步的属性列表</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 在 Actor 组件被初始化后，在 C++ 代码中进行额外的初始化操作。该函数在游戏运行时被调用。
	virtual void PostInitializeComponents() override;
	// 当该对象在服务器上的运动状态发生变化时，客户端会通过该函数收到通知并更新对应的运动状态。
	virtual void OnRep_ReplicatedMovement() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override; //会在所有机器上调用


private://-----------------------------------------------------------------------------------------------------------
	/// <summary>
	/// 相机弹簧杆
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	/// <summary>
	/// 角色跟随相机
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	/// <summary>
	/// 头顶ui
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;

	//UPROPERTY(Replicated)//用于标记某个成员变量需要进行同步复制（Replicated）
	//UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) 用于标记某个成员变量需要进行同步复制，并指定当该变量复制时需要调用的回调函数
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCombatComponent* CombatCmp;
	UPROPERTY(VisibleAnywhere)
		class UBuffComponent* BuffCmp;
	UPROPERTY(VisibleAnywhere)
		class ULagCompensationComponent* LagCompensationCmp;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation; //从跑或跳跃结束后的角色起始目标值

	ETurningInPlace TurningInPlace; //角色转向

	/// <summary>
	/// 武器蒙太奇动画，可在角色蓝图指定
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		class UAnimMontage* FireWeaponMontage;

	/// <summary>
	/// 重新加载弹夹蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* ReloadMagMontage;

	/// <summary>
	/// 受击蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* HitReactMontage;

	/// <summary>
	/// 淘汰蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* ElimMontage;

	/// <summary>
	/// 投掷手榴弹蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* ThrowGrenadeMontage;

	/// <summary>
	/// 交换蒙太奇动画
	/// </summary>
	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* SwapMontage;

	/// <summary>
	/// 相机和角色距离阈值
	/// </summary>
	UPROPERTY(EditAnyWhere)
		float CameraThreshold = 200.f;

	/*  代理角色使用  */
	bool bRotateRootBone = false; //是否旋转根骨骼
	float TurnThreshold = 0.5f; //原地转向的阈值
	FRotator ProxyRotationLastFrame;  //代理上一次的旋转值
	FRotator ProxyRotationCur;
	float ProxyYawOffset = 0.f;
	float TimeSinceLastMovementReplication = 0.f; //上一次代理角色移动组件的网络同步时间

	/*  player health  */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_CurHealth, VisibleAnywhere, Category = "Player Stats")
		float CurHealth = MaxHealth;

	/*  player shield  */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_CurShield, EditAnywhere, Category = "Player Stats")
		float CurShield = 0.f;

	/// <summary>
	/// 是否淘汰
	/// </summary>
	bool bElimmed = false;
	/// <summary>
	/// 淘汰结束倒计时 结束后复活
	/// </summary>
	FTimerHandle ElimTimer;
	/// <summary>
	/// 淘汰计时器时间
	///  EditDefaultsOnly可以在编辑器编辑，但只能在默认值之上
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;

	/*  溶解特效  */
	UPROPERTY(EditAnywhere, Category = Elim)
		UCurveFloat* DissolveCurve;//溶解时间曲线
	UPROPERTY(EditAnywhere)
		UTimelineComponent* DissolveTimelineCmp;
	FOnTimelineFloat DissolveTrack; //处理时间轴（timeline）中浮点数值变化的事件
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMatInstance; //在运行时动态创建的材质实例。它可以用于在游戏或应用程序中即时修改材质的属性，例如改变颜色、纹理、参数等等。
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstance* DissolveMatInstance; //溶解材质实例，适合在需要多次使用相同材质但有不同属性的场景中使用，可在蓝图里使用

	/* team color */
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* OriginalMaterial;
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedDissolveMatInst;
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedMaterial;
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueDissolveMatInst;
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueMaterial;

	/* 淘汰回收机器人特效 */
	UPROPERTY(EditAnywhere, Category = Elim)
		class USoundCue* ElimBotSound;
	UPROPERTY(EditAnywhere, Category = Elim)
		UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UParticleSystemComponent* ElimBotComponent;

	/* 皇冠 */
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* CrownSystem;
	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* CrownComponent;

	/* 手榴弹 */
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachedGrenade;

	/* 出场默认武器 */
	UPROPERTY(EditAnywhere)
		TSubclassOf<AWeapon> DefaultWeaponClass;

	/* 用于服务器倒带的角色命中框box */
	UPROPERTY(EditAnywhere)
		class UBoxComponent* head;
	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvis;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* blanket;
	UPROPERTY(EditAnywhere)
		UBoxComponent* backpack;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_r;
	UPROPERTY()
		TMap<FName, UBoxComponent*> HitConllisionBoxs;

	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;
	UPROPERTY()
		class ABlasterPlayerController* BlasterPlayerController;
	UPROPERTY()
		class ABlasterPlayerState* BlasterPlayerState;

	/// <summary>
	/// 禁止游戏输入
	/// </summary>
	UPROPERTY(Replicated)
		bool bDisableGameplay = false;

	/// <summary>
	/// 退出当前游戏
	/// </summary>
	bool bLeftGame = false;


public:
	/// <summary>
	/// 交换武器动作是否完成
	/// </summary>
	bool bFinishedSwapping = false;

	/// <summary>
	/// 退出当前游戏事件
	/// </summary>
	FOnLeftGame OnLeftGame;


protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipBtnPressed();
	void CrouchBtnPressed();
	void AimBtnPressed();
	void AimBtnReleased();
	virtual void Jump() override;
	void FireBtnPressed();
	void FireBtnReleased();
	void ReloadMagBtnPressed();
	void GrenadeBtnPressed();

	void AimOffset(float DeltaTime); //瞄准偏移
	void CalculateAO_Pitch();
	void SimProxiesTurn();//模拟代理使用的旋转逻辑

	/* 轮询检查玩家数据有效时 初始化hud等工作 */
	void PollInit();
	/* 处理玩家首次初始化并拉取到数据是应该发生的事件 */
	void OnPlayerStateInitialized();
	/* 原地旋转 */
	void RotateInPlace(float DeltaTime);
	/* 丢掉或删除武器 */
	void DropOrDestroyWeapons();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	/* 设置角色出生点 */
	void SetSpawnPoint();

	/// <summary>
	/// 接收伤害回调
	/// 即当角色受到伤害时（UGameplayStatics::ApplyDamage），引擎会自动调用该函数并传递伤害相关的参数，
	/// </summary>
	/// <param name="DamagedActor">被攻击的角色</param>
	/// <param name="Damage">造成的伤害</param>
	/// <param name="DamageType">伤害类型</param>
	/// <param name="InstigatorController">造成伤害的控制器</param>
	/// <param name="DamageCauser">造成伤害的对象</param>
	UFUNCTION()
		void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);


public:
	//设置武器，内联函数
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }
	/// <summary>
	/// 设置与地面上的重叠武器
	/// </summary>
	/// <param name="Weapon"></param>
	void SetOverlappingWeapon(AWeapon* Weapon);
	/// <summary>
	/// 是否装备了武器
	/// </summary>
	/// <returns></returns>
	bool IsWeaponEquipped();
	/// <summary>
	/// 是否正在瞄准
	/// </summary>
	/// <returns></returns>
	bool IsAiming();
	/// <summary>
	/// 是否显示瞄准umg
	/// </summary>
	/// <param name="bShowScope"></param>
	UFUNCTION(BlueprintImplementableEvent) //可蓝图实现函数
		void ShowSniperScopeWidget(bool bShowScope);
	/// <summary>
	/// 出生时生成默认武器
	/// </summary>
	void SpawnDefaultWeapon();

	FVector GetHitTarget() const;
	AWeapon* GetEquippedWeapon();
	ECombatState GetCombatState() const;
	bool GetIsLocallyReloading() const;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMagMontage(); //播放装弹夹动画
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage(); //投掷手榴弹蒙太奇
	void PlaySwapMontage();

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	/*	UFUNCTION(NetMulticast, Unreliable) //** 这里改为由更新角色健康值时触发，健康值会被同步到所有客服端，放到这那里会减少一次网络广播消耗
			void MulticastHit();	*///播放受击动画 NetMulticast会从服务端同步到所有客户端 Unreliable表示同步消息不可靠

			/// <summary>
			/// 淘汰，server上执行
			/// </summary>
			/// <param name="bPlayerLeftGame">是否是退出游戏</param>
	void Elim(bool bPlayerLeftGame);
	/// <summary>
	/// 淘汰网络多播
	/// </summary>
	/// <param name="bPlayerLeftGame">是否是退出游戏</param>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim(bool bPlayerLeftGame);

	/// <summary>
	/// 要离开游戏，通知server
	/// </summary>
	UFUNCTION(Server, Reliable)
		void ServerLeavaGame();

	/// <summary>
	/// 设置第一名的状态
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastGainedTheLead();
	/// <summary>
	/// 失去第一位置
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastLostTheLead();

	// 设置队伍材质颜色
	void SetTeamColor(ETeam Team);
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);

	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetCurHealth() const { return CurHealth; }
	FORCEINLINE void SetCurHealth(const float Amount) { CurHealth = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetCurShield() const { return CurShield; }
	FORCEINLINE void SetCurShield(const float Amount) { CurShield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE void SetDisableGameplay(const bool bDisable) { bDisableGameplay = bDisable; }
	FORCEINLINE UCombatComponent* GetCombatCmp() const { return CombatCmp; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMagMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComp() const { return BuffCmp; }
	FORCEINLINE TMap<FName, UBoxComponent*> GetHitCollisionBoxs() const { return HitConllisionBoxs; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComp() const { return LagCompensationCmp; }
	FORCEINLINE bool IsHoldingTheFlag() const;


private:
	/// <summary>
	/// 当 OverlappingWeapon 变量在客户端上更新时（服务通知客户端更新），将自动调用该回调函数进行处理
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/// <summary>
	/// 同步CurHealth
	/// </summary>
	/// <param name="LastHealth">上一次的血量</param>
	UFUNCTION()
		void OnRep_CurHealth(float LastHealth);
	/// <summary>
	/// 同步护盾值
	/// </summary>
	/// <param name="LastShield">上一次的护盾值</param>
	UFUNCTION()
		void OnRep_CurShield(float LastShield);

	/// <summary>
	/// 在客户端调用该函数时实际上会发送一个 RPC 请求到服务器，请求服务器执行其实现版本=ServerEquipBtnPressed_Implementation。
	/// https://docs.unrealengine.com/5.1/zh-CN/rpcs-in-unreal-engine/
	/// </summary>
	UFUNCTION(Server, Reliable)	//声明为一个要在客户端上调用、但需要在服务器上执行代码的 RPC，并使用可靠的网络传输方式进行通信。
		void ServerEquipBtnPressed();

	void TurnInPlace(float DeltaTime);

	/* 解决角色靠墙时，相机离角色添加而挡住视野，太近时隐藏角色 */
	void HideCameraIfCharacterClose();

	float CalculateSpeed();

	void ElimTimerFinished();

	/// <summary>
	/// 更新溶解材质的参数值
	/// </summary>
	/// <param name="DissloveVal"></param>
	UFUNCTION()
		void UpdataDissloveMaterial(float DissloveVal);
	/// <summary>
	/// 开启溶解
	/// </summary>
	void StartDisslove();

};
