// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTaskInformation.h"
#include "MDownloadEvent.h"
#include "MHttpdownloader.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


/**
 * a download task, normally operated by FileDownloadManager, extreamly advise you to use FileDownloadManager.
 */
class FMDownloadTask
{
public:
	FMDownloadTask();

	FMDownloadTask(const FString& InUrl, const FString& InDirectory, const FString& InFileName);

	FMDownloadTask(const FMTaskInformation& InTaskInfo);

	~FMDownloadTask();


protected:
	FMDownloadTask(const FMDownloadTask& rhs) = delete;	// 禁用拷贝构造函数
	FMDownloadTask& operator=(const FMDownloadTask& rhs) = delete;	// 禁用拷贝赋值运算符


public:
	virtual void SetFileName(const FString& InFileName);
	virtual const FString& GetFileName() const;

	virtual void SetDestDirectory(const FString& InDestDirectory);
	virtual const FString& GetDestDirectory() const;

	virtual void SetSourceUrl(const FString& InSourceUrl);
	virtual const FString& GetSourceUrl() const;

	virtual void SetETag(const FString& InETag);
	virtual const FString& GetETag() const;

	virtual void SetCurrentSize(int32 InCurrentSize);
	virtual int32 GetCurrentSize() const;

	virtual void SetTotalSize(int32 InTotalSize);
	virtual int32 GetTotalSize() const;

	virtual void SetNeedStop(const bool InNeedStop);
	virtual const bool GetNeedStop() const;


public:
	/// <summary>
	/// 获取下载进度百分比
	/// </summary>
	/// <returns></returns>
	virtual int32 GetPercentage() const;
	/// <summary>
	/// 开始下载
	/// </summary>
	/// <returns></returns>
	virtual bool Start();
	/// <summary>
	/// 暂停下载
	/// </summary>
	/// <returns></returns>
	virtual bool Stop();
	/// <summary>
	/// 是否下载中
	/// </summary>
	/// <returns></returns>
	virtual bool IsDownloading() const;
	/// <summary>
	/// 创建guid
	/// </summary>
	void ReGenerateGUID();
	/// <summary>
	/// 保存下载任务到json
	/// </summary>
	/// <param name="InFileName"></param>
	/// <returns></returns>
	bool SaveTaskToJsonFile(const FString& InFileName) const;


public:
	inline int32 GetGUID() const { return TaskInfo.GetGUID(); }
	inline FMTaskInformation GetTaskInformation() const { return TaskInfo; }
	inline EMTaskState GetState() const { return TaskState; }


public:
	// 用于通知下载事件的回调的函数指针
	TFunction<void(EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode)> PregressTaskFunc = [this](EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode)
		{
			// 默认指向的函数逻辑
			if (InEvent == EMTaskEvent::START_DOWNLOAD)
			{
				// __FUNCTION__ 和 __LINE__ 是 C++ 中的预定义宏，用于获取当前代码所在的函数名称和行号。
				UE_LOG(LogFileDownloader,
					Warning, 
					TEXT("%s  %d  Please use FileDownloadManager instead DownloadTask to download file.（请使用FileDownloadManager而不是DownloadTask来下载文件。）"),
					__FUNCTION__,
					__LINE__);
			}
		};


protected:
	/// <summary>
	/// 获取http头部信息
	/// </summary>
	virtual void GetHead();
	/// <summary>
	/// 开始切块
	/// </summary>
	virtual void StartChunk();
	/// <summary>
	/// 获取完整的文件路径
	/// </summary>
	/// <returns></returns>
	virtual FString GetFullFileName() const;

	virtual void OnGetHeadCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWadSuccessful);
	virtual void OnGetChunkCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWadSuccessful);
	
	virtual void OnTaskCompleted();
	virtual void OnWriteChunkEnd(int32 DataSize);


protected:
	FMTaskInformation TaskInfo;
	EMTaskState TaskState = EMTaskState::WAIT;
	int32 ChunkSize = 2 * 1024 * 1024;	// 2MB作为每块的大小
	TArray<uint8> DataBuffer;
	FString EncodedUrl;	// 编码处理后的url
	IFileHandle* TargetFilePtr = nullptr;
	FHttpRequestPtr RequestPtr = nullptr;
	int32 CurrentTypeCount = 0;	// 当前尝试请求的次数
	int32 MaxTryCount = 5;	// 最大请求次数
	bool bNeedStop = false;

};
