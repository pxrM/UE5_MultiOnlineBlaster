// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

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


public:
	/// <summary>
	/// 用于获取需要进行网络同步的属性列表
	/// </summary>
	/// <param name="OutLifetimeProps">需要进行网络同步的属性列表</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//设置武器，内联函数
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }
	void SetOverlappingWeapon(AWeapon* Weapon);


private:
	/// <summary>
	/// 当 OverlappingWeapon 变量在客户端上更新时（服务通知客户端更新），将自动调用该回调函数进行处理
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

};
