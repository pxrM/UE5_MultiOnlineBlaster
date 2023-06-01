// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HttpManagerSettings.generated.h"

/**
 *
 */

/*
  config = Game ��ʾ�����������Ϸ��������ã��ᱣ������Ϸ�����ļ��У�
  defaultconfig ��ʾ������þ���Ĭ��ֵ���ҿ���������༭���н��б༭��
  mate = (DisplayName = "HttpMgrSetting") �������������������༭������ʾ�����ơ�
*/
UCLASS(config = Game, DefaultConfig, DisplayName = "HttpManagerSettings")
class MHTTPDOWNTOOL_API UHttpManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHttpManagerSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 MaxParallel;	//������ز�����
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 MaxTryCount;	//���������
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 RequestKBSize; //ÿ���������С
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		float ResponseTimeout; //����ʱ��ʱ��
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		FString CurFilePath; //���ļ��Ļ���λ��
};