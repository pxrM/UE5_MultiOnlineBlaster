// Fill out your copyright notice in the Description page of Project Settings.
/*
	查看有多少玩家连接到大厅，一旦进入一定数量就可以前往实际的游戏地图
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 登录
	/// </summary>
	/// <param name="NewPlayer">新加入的玩家控制器</param>
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
