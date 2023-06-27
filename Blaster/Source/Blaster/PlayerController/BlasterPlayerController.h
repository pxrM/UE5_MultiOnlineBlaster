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
	//��һ����������ȡһ�� Pawn ���������øÿ������� OnPossess ������������ȡ�� Pawn ��Ϊ�����������С�
	virtual void OnPossess(APawn* InPawn)override;
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual float GetServerTime(); //�������ʱ��ͬ��
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void OnMatchStateSet(FName State);


protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/*
		�������Ϳͻ���֮���ʱ��ͬ��
	*/
	/// <summary>
	/// ʹ��RPC����Ϊ�Ƿ�����rpc��������ǰ����˸�Server�����������ʱ�䣬�ú��������ڷ�������
	/// </summary>
	/// <param name="TimeOfClientRequest">�ͻ��˷��������ʱ��</param>
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	/// <summary>
	/// ��ServerRequestServerTime�������Ӧ����÷�����ʱ�䣬�ú��������ڿͻ�����
	/// </summary>
	/// <param name="TimeOfClientRequest">�ͻ��˷��������ʱ��</param>
	/// <param name="TimeServerReceivedRequest">���������յ��ͻ��������ʱ��</param>
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedRequest);
	/// <summary>
	/// ����Ƿ�Ҫ����һ��ʱ��ͬ��
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CheckTimeSync(float DeltaTime);
	/// <summary>
	/// ��ѯ����ʼ����ɫ״̬HUD
	/// </summary>
	void PollInit();

	/*
	 ��Ϸƥ��״̬���	
	*/
	/// <summary>
	/// �����������Ϸƥ��״̬
	/// </summary>
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();
	/// <summary>
	/// �ͻ��˼���ʱ֪ͨһ����Ϸ״̬
	/// </summary>
	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);
	/// <summary>
	/// ������ʼ����
	/// </summary>
	void HandleMatchHasStarted();
	/// <summary>
	/// ����������ȴ�׶�����
	/// </summary>
	void HandleCooldown();


private:
	UFUNCTION()
		void OnRep_MatchState();


protected:
	/// <summary>
	/// �ͻ��˺ͷ�����֮���ʱ�����
	/// </summary>
	float ClientServerDelte = 0.f;
	/// <summary>
	/// ʱ��ͬ��Ƶ��
	/// </summary>
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 5.f;
	/// <summary>
	/// ʱ��ͬ������ʱ��
	/// </summary>
	float TimeSyncRunningTime = 0.f;


private:
	UPROPERTY()
		class ABlasterHUD* BlasterHUD;
	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWidget;
	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefaults;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState; // ƥ��״̬
	float LevelStartingTime = 0.f;	// �ؿ���ʼʱ�䣬ÿ����ҽ���ؿ���ʱ�䲻һ���������Է�����Ϊ׼
	float MatchTime = 0.f;	// ����ʱ������gamemode�л�ȡ
	float WarmupTime = 0.f;	 // Ԥ��ʱ��
	float CooldownTime = 0.f; // ������ȴʱ��
	uint32 CountdownInt = 0;  // ��һ�ε���ʱ��ʱ�䣬����뵱ǰ��ͬ�����hud

};
