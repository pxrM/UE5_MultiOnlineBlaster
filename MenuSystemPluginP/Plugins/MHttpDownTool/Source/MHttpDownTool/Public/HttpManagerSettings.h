// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HttpManagerSettings.generated.h"

/**
 *
 */

/*
  config = Game 表示这个设置是游戏级别的配置，会保存在游戏配置文件中；
  defaultconfig 表示这个设置具有默认值并且可以在引擎编辑器中进行编辑；
  mate = (DisplayName = "HttpMgrSetting") 则定义了这个设置在引擎编辑器中显示的名称。
*/
UCLASS(config = Game, DefaultConfig, DisplayName = "HttpManagerSettings")
class MHTTPDOWNTOOL_API UHttpManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHttpManagerSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 MaxParallel;	//最大下载并行数
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 MaxTryCount;	//最大重连数
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		int32 RequestKBSize; //每个子任务大小
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		float ResponseTimeout; //请求超时的时间
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
		FString CurFilePath; //子文件的缓存位置
};