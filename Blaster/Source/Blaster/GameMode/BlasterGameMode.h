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
	/// 淘汰角色
	/// </summary>
	/// <param name="ElimmedCharacter">被淘汰的角色</param>
	/// <param name="VictimController">受害者的控制器</param>
	/// <param name="AttackerController">加害者的控制器</param>
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	/// <summary>
	/// 请求复活
	/// </summary>
	/// <param name="ElimmedCharacter">被淘汰的角色</param>
	/// <param name="ElimmedController">被淘汰的控制器</param>
	virtual void ResquestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);


protected:
	virtual void BeginPlay() override;


public:
	/// <summary>
	/// 预热时间，结束后开始StartMatch
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	/// <summary>
	/// 关卡开始时间
	/// </summary>
	float LevelStartingTime = 0.f;


private:
	float CountdownTime = 0.f; //预热倒计时

};
