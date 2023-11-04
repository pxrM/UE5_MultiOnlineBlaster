// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 占领旗帜模式:
 *	红蓝队各有自己队伍的旗帜区，玩家从对方区域夺得旗帜并带回自己队伍的旗帜加分，加分后该旗帜消失回到原点
 */
UCLASS()
class BLASTER_API ACaptureTheFlagGameMode : public ATeamGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	/// <summary>
	/// server处理加分
	/// </summary>
	/// <param name="Flag"></param>
	/// <param name="Zone"></param>
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);

};
