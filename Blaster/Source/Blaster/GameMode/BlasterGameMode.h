// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime);

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


protected:
	virtual void BeginPlay() override;


public:
	/// <summary>
	/// Ԥ��ʱ�䣬������ʼStartMatch
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	/// <summary>
	/// �ؿ���ʼʱ��
	/// </summary>
	float LevelStartingTime = 0.f;


private:
	float CountdownTime = 0.f; //Ԥ�ȵ���ʱ

};
