// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation()override;
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;


private:
	/*
	UPROPERTY 表示这个属性是由 Unreal Engine 管理的 Unreal Object System 中的属性。它可以让 Unreal Engine 在编译时自动生成一些辅助代码，
				例如序列化（Serialization），或者在编辑器中显示这个属性以便于修改。
	BlueprintReadOnly 表示这个属性只能在蓝图（Blueprint）中读取，而不能写入。这个属性通常用于将 C++ 类型的数据暴露给蓝图使用，
				但是又不希望蓝图直接修改这个数据，而是通过调用 C++ 函数来改变。
	Category = Character 表示这个属性属于 Character 类别，是用于描述角色属性的。在 Unreal Engine 中，所有的属性都需要被分配到一个所属类别（Category）中，
				以便于在编辑器中进行分类和组织。其中一些常用的类别已经被 Unreal Engine 预定义好了，例如 Character、Engine、Input 等等。
				在 C++ 代码中声明的 UPROPERTY 宏中，可以使用 Category 参数将属性分配到预定义的类别中，也可以创建自定义的类别来管理属性。
				如果使用未定义的类别，它会在编辑器中显示为一个新的类别，但是不会影响代码的编译和运行。因此，Category = Character 可能是预定义的 Character 类别，
				也可能是开发者自己定义的 Character 类别。
	meta = (AllowPrivateAccess = "true") 表示这个属性允许私有访问（AllowPrivateAccess），也就是说，即使它是私有成员变量，
				蓝图也可以访问并读取这个属性。这个属性通常用于在蓝图中访问 C++ 代码中的私有成员变量。
	*/

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class ABlasterCharacter* BlasterCharacter;	//角色

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Speed;  //速度

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;	//是否在空中

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;	//是否正在加速移动

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;	//是否装备了武器
	UPROPERTY()
		class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouched;	//是否蹲下

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bIsAiming;	  //是否在瞄准

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		float YawOffset;	  //控制混合动画EquippedRun y

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		float Lean;	  //控制混合动画EquippedRun x

	FRotator DeltaRotation;
	FRotator CharacterRotation;	//当前帧的旋转角度值
	FRotator CharacterRotationLastFrame; //上一帧的旋转角度值

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		float AO_Yaw; //瞄准偏移量 左右

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		float AO_Pitch; //瞄准偏移量 上下

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		FTransform LeftHandTransform;	//左手的变换位置，抓枪用

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		FRotator RightHandRotation;	//右手骨骼旋转

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bLocalControlled; //是否是本地控制器

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bRotateRootBone; //是否旋转根骨骼

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bElimmed; //是否淘汰

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bUseFABRIK; //是否启用左手ik

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bUseAimOffsets; //是否使用瞄准偏移

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bTransformRightHand; //是否启用右手变换

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bHoldingTheFlag; //是否持有旗帜

};
