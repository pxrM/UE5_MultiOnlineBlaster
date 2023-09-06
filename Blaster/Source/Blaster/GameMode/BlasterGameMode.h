// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//在MatchState命名空间下添加自己的状态
namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	//比赛时间已结束，显示获胜者并开始冷却倒计时
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
	/// <summary>
	/// 角色请求退出游戏的处理
	/// </summary>
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);


public:
	FORCEINLINE float GetCountdownTime() { return CountdownTime; }


public:
	/// <summary>
	/// 关卡开始时间
	/// </summary>
	float LevelStartingTime = 0.f;
	/// <summary>
	/// 预热时间，结束后开始StartMatch
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	/// <summary>
	/// 比赛时长
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;
	/// <summary>
	/// 冷却时长
	/// </summary>
	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;


private:
	/// <summary>
	/// 当前时长的倒计时时间
	/// </summary>
	float CountdownTime = 0.f;

};
