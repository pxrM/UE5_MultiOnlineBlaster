// Fill out your copyright notice in the Description page of Project Settings.
/*
	网络延迟补偿组件
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


/// <summary>
/// 由于不能直接存box的指针，因为指针指向的是地址，所以需要单独的数据结构存储盒子的信息
/// </summary>
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 位置
	/// </summary>
	UPROPERTY()
		FVector Location;
	/// <summary>
	/// 旋转
	/// </summary>
	UPROPERTY()
		FRotator Rotation;
	/// <summary>
	/// 盒子范围
	/// </summary>
	UPROPERTY()
		FVector BoxExtent;
};


/// <summary>
/// 角色桢数据
/// </summary>
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 存储时间
	/// </summary>
	UPROPERTY()
		float Time;
	/// <summary>
	/// 角色不同部位所对应的box信息
	/// </summary>
	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfo;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;
public:
	// Sets default values for this component's properties
	ULagCompensationComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;


private:
	void SaveFramePackage(FFramePackage& Package);

public:
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);

private:
	UPROPERTY()
		ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;


};
