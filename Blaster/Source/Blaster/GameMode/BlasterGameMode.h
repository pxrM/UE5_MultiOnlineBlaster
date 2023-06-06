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
	/// <summary>
	/// 淘汰角色
	/// </summary>
	/// <param name="ElimmedCharacter">被淘汰的角色</param>
	/// <param name="VictimController">受害者的控制器</param>
	/// <param name="AttackerController">加害者的控制器</param>
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
};
