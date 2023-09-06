// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

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

	virtual float GetServerTime(); //与服务器时间同步
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void OnMatchStateSet(FName State);


protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void SetHUDTime();

	/*
		服务器和客户端之间的时间同步
	*/
	/// <summary>
	/// 使用RPC（因为是服务器rpc函数所以前面加了个Server）请求服务器时间，该函数运行在服务器上
	/// </summary>
	/// <param name="TimeOfClientRequest">客户端发送请求的时间</param>
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	/// <summary>
	/// 对ServerRequestServerTime请求的响应，获得服务器时间，该函数运行在客户端上
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
	/// 轮询检查初始化角色状态HUD
	/// </summary>
	void PollInit();

	/*
	 游戏匹配状态相关
	*/
	/// <summary>
	/// 服务器检查游戏匹配状态
	/// </summary>
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();
	/// <summary>
	/// 客户端加入时通知一次游戏状态
	/// </summary>
	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);
	/// <summary>
	/// 比赛开始设置
	/// </summary>
	void HandleMatchHasStarted();
	/// <summary>
	/// 比赛结束冷却阶段设置
	/// </summary>
	void HandleCooldown();

	void CheckPing(float DeltaTime);
	void HighPingWarning();
	void StopHigtPingWarning();
	UFUNCTION(Server, Reliable)
		void ServerReportPingStatus(bool bHighPing); //向server发送报告ping状态

	void ShowReturnToMainMenu();


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
	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWidget;

	bool bInitializeHealth = false;
	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeShield = false;
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeScore = false;
	float HUDScore;
	bool bInitializeDefaults = false;
	int32 HUDDefaults;
	bool bInitializeGrenades = false;
	int32 HUDGrenades;
	bool bInitializeCarriedAmmo = false;
	int32 HUDCarriedAmmo;
	bool bInitializeWeaponAmmo = false;
	int32 HUDWeaponAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState; // 匹配状态
	float LevelStartingTime = 0.f;	// 关卡开始时间，每个玩家进入关卡的时间不一样，所以以服务器为准
	float MatchTime = 0.f;	// 比赛时长，从gamemode中获取
	float WarmupTime = 0.f;	 // 预热时长
	float CooldownTime = 0.f; // 比赛冷却时长
	uint32 CountdownInt = 0;  // 上一次倒计时的时间，如果与当前不同则更新hud

	float HighPingRunningTime = 0.f;
	float PingAnimRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
		float CheckPingFrequency = 20.f; //间隔多久检测一次ping
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 50.f; //超过这个值为高ping
	UPROPERTY(EditAnywhere)
		float HighPingDuration = 5.f; //高ping的一次展示持续时间

	/*
	* 返回主界面ui
	*/
	UPROPERTY(EditAnywhere, Category = HUD)
		TSubclassOf<class UUserWidget> ReturnToMainWidget;
	UPROPERTY()
		class UReturnToMainMenu* ReturnToMainMenu;
	bool bReturnToMainMenuOpen = false;


public:
	/// <summary>
	/// rpc单程发送时间
	/// </summary>
	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

};
