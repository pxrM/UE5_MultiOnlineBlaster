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

/// <summary>
/// 服务器倒带命中结果
/// </summary>
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
public:
	/// <summary>
	/// 是否命中
	/// </summary>
	UPROPERTY()
		bool bHitConfirmed;
	/// <summary>
	/// 是否爆头
	/// </summary>
	UPROPERTY()
		bool bHeadShot;
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
	/// <summary>
	/// 缓存每一帧的box数据包
	/// </summary>
	void SaveFramePackage();
	/// <summary>
	/// 缓存一帧的数据包
	/// </summary>
	/// <param name="Package"></param>
	void SaveFramePackage(FFramePackage& Package);
	/// <summary>
	/// 服务器射击倒带
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocation">命中位置</param>
	/// <param name="HitTime">命中时间</param>
	/// <returns>命中结果</returns>
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);
	/// <summary>
	/// 在命中时间的前一帧和后一帧之间进行插值运算
	/// </summary>
	/// <param name="OlderFrmae"></param>
	/// <param name="YoungerFrame"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrmae, const FFramePackage& YoungerFrame, float HitTime);
	/// <summary>
	/// 计算命中结果
	/// </summary>
	/// <param name="Package">倒带出来的结果包</param>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocaton">命中位置</param>
	/// <returns></returns>
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package, 
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocaton);
	/// <summary>
	/// 缓存角色当前box的位置
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="OutFrameackage"></param>
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFrameackage);
	/// <summary>
	/// 移动命中角色的box到指定时间的位置
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// 恢复命中角色box的位置到最新位置
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// 设置角色网格的碰撞是否开启
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="CollisionEnabled"></param>
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	

public:
	/// <summary>
	/// debug 显示数据的box框
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);
	/// <summary>
	/// 向服务器请求攻击结果，获取分数
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocation">命中位置</param>
	/// <param name="HitTime">命中时间</param>
	/// <param name="DamageCauser">伤害原因</param>
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			class ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime,
			class AWeapon* DamageCauser);


private:
	UPROPERTY()
		ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	/// <summary>
	/// 存储4秒的帧数据，时间太长和太短都会影响体验
	/// </summary>
	UPROPERTY(EditAnywhere)
		float MaxRecordTime = 4.f;
	/// <summary>
	/// 存储《MaxRecordTime》时间内发生的帧数据，使用双向链表方便从链表头尾节点执行添加和移除操作
	/// </summary>
	TDoubleLinkedList<FFramePackage> FrameHistory;

};
