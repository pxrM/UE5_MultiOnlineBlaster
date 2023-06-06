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
	/// ��̭��ɫ
	/// </summary>
	/// <param name="ElimmedCharacter">����̭�Ľ�ɫ</param>
	/// <param name="VictimController">�ܺ��ߵĿ�����</param>
	/// <param name="AttackerController">�Ӻ��ߵĿ�����</param>
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
};
