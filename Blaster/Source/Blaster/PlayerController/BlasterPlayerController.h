// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//当一个控制器获取一个 Pawn 后，引擎会调用该控制器的 OnPossess 函数，并将获取的 Pawn 作为参数传入其中。
	virtual void OnPossess(APawn* InPawn)override;
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual float GetServerTime(); //与服务器世界时间同步
	void OnMatchStateSet(FName State);


protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/*
		服务器和客户端之间的时间同步
	*/
	/// <summary>
	/// 使用RPC（因为是服务器rpc函数所以前面加了个Server）请求服务器时间
	/// </summary>
	/// <param name="TimeOfClientRequest">客户端发送请求的时间</param>
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	/// <summary>
	/// 对ServerRequestServerTime请求的响应，获得服务器时间
	/// </summary>
	/// <param name="TimeOfClientRequest">客户端发送请求的时间</param>
	/// <param name="TimeServerReceivedRequest">服务器接收到客户端请求的时间</param>
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedRequest);
	/// <summary>
	/// 检查是否要进行一次时间同步
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CheckTimeSync(float DeltaTime);
	/// <summary>
	/// 轮询检查初始化
	/// </summary>
	void PollInit();


private:
	UFUNCTION()
		void OnRep_MatchState();


protected:
	/// <summary>
	/// 客户端和服务器之间的时间差异
	/// </summary>
	float ClientServerDelte = 0.f;
	/// <summary>
	/// 时间同步频率
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 5.f;
	/// <summary>
	/// 时间同步运行时间
	/// </summary>
	float TimeSyncRunningTime = 0.f;


private:
	UPROPERTY()
		class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState; //匹配状态

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWidget;
	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefaults;


};
