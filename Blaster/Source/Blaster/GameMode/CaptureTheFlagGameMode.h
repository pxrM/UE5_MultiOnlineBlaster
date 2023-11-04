// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * ռ������ģʽ:
 *	�����Ӹ����Լ����������������ҴӶԷ����������Ĳ������Լ���������ļӷ֣��ӷֺ��������ʧ�ص�ԭ��
 */
UCLASS()
class BLASTER_API ACaptureTheFlagGameMode : public ATeamGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	/// <summary>
	/// server����ӷ�
	/// </summary>
	/// <param name="Flag"></param>
	/// <param name="Zone"></param>
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);

};
