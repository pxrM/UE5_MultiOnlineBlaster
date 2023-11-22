// Fill out your copyright notice in the Description page of Project Settings.


/**
* 
* https://space.bilibili.com/43501343
* 
* HTTP/1.1 规范：https://tools.ietf.org/html/rfc7230
* HTTP/1.1 请求方法：https://tools.ietf.org/html/rfc7231#section-4
* HTTP/1.1 响应状态码：https://tools.ietf.org/html/rfc7231#section-6
* HTTP/1.1 首部字段：https://tools.ietf.org/html/rfc7231#section-5
* 
* 
*/


#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "HttpMgrSubsystem.generated.h"


#define M_SUFFIX TEXT("hcf")//下载后文件后缀 hcf  =  http cache file

/**
 *
 */
UCLASS()
class MHTTPDOWNTOOL_API UHttpMgrSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId()const { return TStatId(); }

	//创建子任务
	class UAsyncDownloadFile* CreateDownTask(
		UObject* WorldContextObject,
		const FString& Url,
		const FString& FileName,
		const FString& Directory,
		bool bClearCache
	);

	void ExecDownloadTasks();

	void NotifyFailed(UAsyncDownloadFile* HttpFile, bool bReconnection);

	void NotifyFinised(UAsyncDownloadFile* HttpFile);

	void ClearDownloadHttp();


private:
	UPROPERTY()
		TArray<UAsyncDownloadFile*> DownloadHttpFiles;

	void Init();

public:
	int32 MaxParallel;
	int32 MaxTryCount;
	int32 RequestKBSize;
	float ReponseTimeout;
	FString CurFilePath;

};
