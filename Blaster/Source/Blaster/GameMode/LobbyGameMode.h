// Fill out your copyright notice in the Description page of Project Settings.
/*
	�鿴�ж���������ӵ�������һ������һ�������Ϳ���ǰ��ʵ�ʵ���Ϸ��ͼ
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
	/// ��¼
	/// </summary>
	/// <param name="NewPlayer">�¼������ҿ�����</param>
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
