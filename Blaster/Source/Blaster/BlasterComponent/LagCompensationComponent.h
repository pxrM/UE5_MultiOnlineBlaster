// Fill out your copyright notice in the Description page of Project Settings.
/*
	网络延迟补偿组件
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


/**
 * 由于不能直接存box的指针，因为指针指向的是地址，所以需要单独的数据结构存储盒子的信息
 */
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
public:
	/**
	 * 位置
	 */
	UPROPERTY()
	FVector Location;
	/**
	 * 旋转
	 */
	UPROPERTY()
	FRotator Rotation;
	/**
	 * 盒子范围
	 */
	UPROPERTY()
	FVector BoxExtent;
};

/**
 * 角色桢数据
 */
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
public:
	/**
	 * 存储时间
	 */
	UPROPERTY()
	float Time;
	/**
	 * 角色不同部位所对应的box信息
	 */
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
	/**
	 * 角色指针
	 */
	UPROPERTY()
	ABlasterCharacter* Character;
};

/**
 * 服务器倒带命中结果
 */
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
public:
	/**
	 * 是否命中
	 */
	UPROPERTY()
	bool bHitConfirmed;
	/**
	 * 是否爆头
	 */
	UPROPERTY()
	bool bHeadShot;
};

/**
 * 霰弹枪）服务器倒带命中结果
 */
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()
public:
	/**
	 * 每个角色的头部击中次数
	 */
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;
	/**
	 * 每个角色的身体击中次数
	 */
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;
};


/*
 * 滞后补偿 （倒带）
 */
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
	/**
	 * 缓存一帧的数据包
	 */
	void SaveFramePackage(FFramePackage& Package);
	/// <summary>
	/// 根据命中时间获取需要检测的帧数据包
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="HitTime">命中时间</param>
	/// <returns></returns>
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);
	/// <summary>
	/// 在命中时间的前一帧和后一帧之间进行插值运算
	/// </summary>
	/// <param name="OlderFrmae"></param>
	/// <param name="YoungerFrame"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrmae, const FFramePackage& YoungerFrame, float HitTime);
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

	/*
		射线类武器倒带处理
	*/
	/// <summary>
	/// 服务器射击倒带
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocation">命中位置</param>
	/// <param name="HitTime">命中时间</param>
	/// <returns>命中结果</returns>
	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);
	/// <summary>
	/// 计算命中结果（射线）
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
		const FVector_NetQuantize& HitLocation);

	/*
		弹射类武器倒带处理
	*/
	/// <summary>
	/// 弹射类武器倒带处理
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart">开始射击方向</param>
	/// <param name="InitialVelocity">初始速度</param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);
	/// <summary>
	/// 计算命中结果（弹射路径）
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart"></param>
	/// <param name="InitialVelocity"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	/*
		霰弹枪倒带处理
	*/
	/// <summary>
	/// 服务器射击倒带
	/// </summary>
	/// <param name="HitCharacters">多个命中角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocations">多个命中位置</param>
	/// <param name="HitTime">命中时间</param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	/// <summary>
	/// 计算命中结果（多射线）
	/// </summary>
	/// <param name="HitCharacters"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);


public:
	/// <summary>
	/// debug 显示数据的box框
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);

	/// <summary>
	/// （射线类武器）向服务器请求攻击结果，获取分数
	/// </summary>
	/// <param name="HitCharacter">击中的角色</param>
	/// <param name="TraceStart">射击开始位置</param>
	/// <param name="HitLocation">命中位置</param>
	/// <param name="HitTime">命中时间</param>
	/// <param name="DamageCauser">伤害原因</param>
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);
	/// <summary>
	/// （射弹类武器）向服务器请求攻击结果，获取分数
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart"></param>
	/// <param name="InitialVelocity"></param>
	/// <param name="HitTime"></param>
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
	/// <summary>
	/// （霰弹枪武器）向服务器请求攻击结果，获取分数
	/// </summary>
	/// <param name="HitCharacters"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <param name="HitTime"></param>
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);


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
