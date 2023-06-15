// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override; //会在所有机器上调用
	//-----------------------------------------------------------------------------------------------------------

private:
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

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatCmp;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation; //从跑或跳跃结束后的角色起始目标值

	ETurningInPlace TurningInPlace; //角色转向

	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		class UAnimMontage* FireWeaponMontage;	//武器蒙太奇动画，可在角色蓝图指定

	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* ReloadMagMontage; //重新加载弹夹蒙太奇动画

	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* HitReactMontage; //受击蒙太奇动画

	UPROPERTY(EditAnyWhere, Category = CombatMontage)
		UAnimMontage* ElimMontage; //淘汰蒙太奇动画

	UPROPERTY(EditAnyWhere)
		float CameraThreshold = 200.f; //相机和角色距离阈值

	/*  代理角色使用  */
	bool bRotateRootBone;	//是否旋转根骨骼
	float TurnThreshold = 0.5f; //原地转向的阈值
	FRotator ProxyRotationLastFrame;  //代理上一次的旋转值
	FRotator ProxyRotationCur;
	float ProxyYawOffset;
	float TimeSinceLastMovementReplication; //上一次代理角色移动组件的网络同步时间

	/*  player health  */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f; //最大健康值
	UPROPERTY(ReplicatedUsing = OnRep_CurHealth, VisibleAnywhere, Category = "Player Stats")
		float CurHealth = MaxHealth;
	bool bElimmed = false;  //是否淘汰

	FTimerHandle ElimTimer; //淘汰结束倒计时 结束后复活
	UPROPERTY(EditDefaultsOnly) //EditDefaultsOnly可以在编辑器编辑，但只能在默认值之上
		float ElimDelay = 3.f; //淘汰计时器时间

	/*  溶解特效  */
	UPROPERTY(EditAnywhere, Category = Elim)
		UCurveFloat* DissolveCurve; //溶解时间曲线
	UPROPERTY(EditAnywhere)
		UTimelineComponent* DissolveTimelineCmp;
	FOnTimelineFloat DissolveTrack; //处理时间轴（timeline）中浮点数值变化的事件
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMatInstance; //在运行时动态创建的材质实例。它可以用于在游戏或应用程序中即时修改材质的属性，例如改变颜色、纹理、参数等等。
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMatInstance; //适合在需要多次使用相同材质但有不同属性的场景中使用，可在蓝图里使用

	/* 淘汰回收机器人 */
	UPROPERTY(EditAnywhere, Category = Elim)
		class USoundCue* ElimBotSound;
	UPROPERTY(EditAnywhere, Category = Elim)
		UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UParticleSystemComponent* ElimBotComponent;

	UPROPERTY()
		class ABlasterPlayerController* BlasterPlayerController;
	UPROPERTY()
		class ABlasterPlayerState* BlasterPlayerState;


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

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn(); //模拟代理使用

	//接收伤害回调
	//即当角色受到伤害时（UGameplayStatics::ApplyDamage），引擎会自动调用该函数并传递伤害相关的参数，
	//被攻击的角色DamagedActor、造成的伤害Damage、伤害类型DamageType、造成伤害的控制器InstigatorController和造成伤害的对象DamageCauser。
	UFUNCTION()
		void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();

	void PollInit(); // 玩家数据有效时 初始化hud等工作


public:
	/// <summary>
	/// 用于获取需要进行网络同步的属性列表
	/// </summary>
	/// <param name="OutLifetimeProps">需要进行网络同步的属性列表</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 在 Actor 组件被初始化后，在 C++ 代码中进行额外的初始化操作。该函数在游戏运行时被调用。
	virtual void PostInitializeComponents() override;
	//设置武器，内联函数
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }
	void SetOverlappingWeapon(AWeapon* Weapon);
	//是否装备了武器
	bool IsWeaponEquipped();
	//是否正在瞄准
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }

	void PlayFireMontage(bool bAiming);
	void PlayReloadMagMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/*	UFUNCTION(NetMulticast, Unreliable) //** 这里改为由更新角色健康值时触发，健康值会被同步到所有客服端，放到这那里会减少一次网络广播消耗
			void MulticastHit();	*///播放受击动画 NetMulticast会从服务端同步到所有客户端 Unreliable表示同步消息不可靠

			//当该对象在服务器上的运动状态发生变化时，客户端会通过该函数收到通知并更新对应的运动状态。
	virtual void OnRep_ReplicatedMovement() override;

	void Elim();
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim(); //淘汰
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetCurHealth() const { return CurHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }


private:
	/// <summary>
	/// 当 OverlappingWeapon 变量在客户端上更新时（服务通知客户端更新），将自动调用该回调函数进行处理
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/// <summary>
	/// 同步CurHealth
	/// </summary>
	UFUNCTION()
		void OnRep_CurHealth();

	/// <summary>
	/// 在客户端调用该函数时实际上会发送一个 RPC 请求到服务器，请求服务器执行其实现版本=ServerEquipBtnPressed_Implementation。
	/// https://docs.unrealengine.com/5.1/zh-CN/rpcs-in-unreal-engine/
	/// </summary>
	UFUNCTION(Server, Reliable)	//声明为一个要在客户端上调用、但需要在服务器上执行代码的 RPC，并使用可靠的网络传输方式进行通信。
		void ServerEquipBtnPressed();

	void TurnInPlace(float DeltaTime);

	void HideCameraIfCharacterClose(); //解决角色靠墙时，相机离角色添加而挡住视野，太近时隐藏角色

	float CalculateSpeed();

	void ElimTimerFinished();

	UFUNCTION()
		void UpdataDissloveMaterial(float DissloveVal);
	void StartDisslove(); //开启溶解

};
