// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
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


protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipBtnPressed();
	void CrouchBtnPressed();
	void AimBtnPressed();
	void AimBtnReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireBtnPressed();
	void FireBtnReleased();


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

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }

private:
	/// <summary>
	/// 当 OverlappingWeapon 变量在客户端上更新时（服务通知客户端更新），将自动调用该回调函数进行处理
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/// <summary>
	/// 在客户端调用该函数时实际上会发送一个 RPC 请求到服务器，请求服务器执行其实现版本=ServerEquipBtnPressed_Implementation。
	/// https://docs.unrealengine.com/5.1/zh-CN/rpcs-in-unreal-engine/
	/// </summary>
	UFUNCTION(Server, Reliable)	//声明为一个要在客户端上调用、但需要在服务器上执行代码的 RPC，并使用可靠的网络传输方式进行通信。
		void ServerEquipBtnPressed();

	void TurnInPlace(float DeltaTime);

};
