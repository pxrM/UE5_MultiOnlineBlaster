// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//��MatchState�����ռ�������Լ���״̬
namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	//����ʱ���ѽ�������ʾ��ʤ�߲���ʼ��ȴ����ʱ
}

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;


public:
	/// <summary>
	/// ��̭��ɫ
	/// </summary>
	/// <param name="ElimmedCharacter">����̭�Ľ�ɫ</param>
	/// <param name="VictimController">�ܺ��ߵĿ�����</param>
	/// <param name="AttackerController">�Ӻ��ߵĿ�����</param>
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	/// <summary>
	/// ���󸴻�
	/// </summary>
	/// <param name="ElimmedCharacter">����̭�Ľ�ɫ</param>
	/// <param name="ElimmedController">����̭�Ŀ�����</param>
	virtual void ResquestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	/// <summary>
	/// ��ɫ�����˳���Ϸ�Ĵ���
	/// </summary>
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);


public:
	FORCEINLINE float GetCountdownTime() { return CountdownTime; }


public:
	/// <summary>
	/// �ؿ���ʼʱ��
	/// </summary>
	float LevelStartingTime = 0.f;
	/// <summary>
	/// Ԥ��ʱ�䣬������ʼStartMatch
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	/// <summary>
	/// ����ʱ��
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;
	/// <summary>
	/// ��ȴʱ��
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;


private:
	/// <summary>
	/// ��ǰʱ���ĵ���ʱʱ��
	/// </summary>
	float CountdownTime = 0.f;

};
