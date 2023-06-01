// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "AsyncDownloadFile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpDownloadDelegate, const FString&, DesPathOrMsg, float, Progress);

UENUM(BlueprintType)
enum class ETaskState :uint8
{
	_Ready,			//准备
	_Downloading,	//下载中
	_Success,		//成功
	_Retry,			//重新连接
	_Failed,		//失败
};

/**
 *	UBlueprintAsyncActionBase  蓝图异步任务基类
 */
UCLASS()
class MHTTPDOWNTOOL_API UAsyncDownloadFile : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

		friend class UHttpMgrSubsystem;

public:
	// FObjectInitializer用于在创建 UObject 或其派生类型的实例时初始化其属性。它通常用于在构造函数中初始化成员变量
	UAsyncDownloadFile(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader", meta = (WorldContext = "WorldContextObject"))
		static UAsyncDownloadFile* DownLoadHttpFile(
			UObject* WorldContextObject,
			const FString& InUrl,
			const FString& InDirectory,
			const FString& InFileName,
			bool bClearCache = false
		);


protected:
	void StartDownload();	//开始下载
	void RequestFileSize();	 //请求文件大小
	void HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);  //请求获得响应之后对其进行处理的回调函数

	void StartSubTaskDownload(TSharedRef<class FHttpDownloadSubTask> SubTaskRef);
	void HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 TaskId);
	void UpdateProgress();


private:
	// 广播任务失败
	void NotifyFailed();
	// 广播任务等待
	void NotifyWait();
	// 分配子任务
	void AllocationSubTask();
	// 创建子任务
	void CreateSubTask();
	// 更新子任务
	void UpdateTask();
	// 文件下载文成
	void OnFinished();
	// 处理请求超时的子任务
	void CheckDeadTask(double CurTime);
	// 保存文件
	void SaveToFile();
	// 发生错误
	void FatalErr(const FString& ErrMsg);


public:
	UPROPERTY(BlueprintAssignable)
		FHttpDownloadDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FHttpDownloadDelegate OnWait;

	UPROPERTY(BlueprintAssignable)
		FHttpDownloadDelegate OnFail;

	UPROPERTY(BlueprintAssignable)
		FHttpDownloadDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
		FHttpDownloadDelegate OnCancel;


protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		ETaskState State;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		int32 TryCount;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString Url;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString FileName;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString Directory;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		float Progerss;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		int32 CurFileSize;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		int64 TotalFileSize;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		int32 MaxTask;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString MD5Str;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		bool bClearCache;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		int32 WaitResponse;	 //等待响应

	UPROPERTY()
		class UHttpMgrSubsystem* HttpMgr;

	TSharedPtr<IHttpRequest> FileSizeRequest;

	TArray<TSharedPtr<FHttpDownloadSubTask>> SubTaskPtrs;

};
