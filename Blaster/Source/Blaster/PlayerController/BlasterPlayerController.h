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

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual float GetServerTime(); //�����������ʱ��ͬ��
	void OnMatchStateSet(FName State);


protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/*
		�������Ϳͻ���֮���ʱ��ͬ��
	*/
	/// <summary>
	/// ʹ��RPC����Ϊ�Ƿ�����rpc��������ǰ����˸�Server�����������ʱ��
	/// </summary>
	/// <param name="TimeOfClientRequest">�ͻ��˷��������ʱ��</param>
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	/// <summary>
	/// ��ServerRequestServerTime�������Ӧ����÷�����ʱ��
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
	/// ��ѯ����ʼ��
	/// </summary>
	void PollInit();


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

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState; //ƥ��״̬

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWidget;
	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefaults;


};
