// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial UMETA(DisplayName = "初始"),
	EWS_Equipped UMETA(DisplayName = "装备"),
	EWS_Dropped UMETA(DisplayName = "掉落"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;


private:
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
	/// umg
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


public:
	void ShowPickupWidget(bool bShow);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }


private:
	UFUNCTION()
		void OnRep_WeaponState();

};
