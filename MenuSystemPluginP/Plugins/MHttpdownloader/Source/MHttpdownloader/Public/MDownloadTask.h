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
	FMDownloadTask(const FMDownloadTask& rhs) = delete;	// ���ÿ������캯��
	FMDownloadTask& operator=(const FMDownloadTask& rhs) = delete;	// ���ÿ�����ֵ�����


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
	/// ��ȡ���ؽ��Ȱٷֱ�
	/// </summary>
	/// <returns></returns>
	virtual int32 GetPercentage() const;
	/// <summary>
	/// ��ʼ����
	/// </summary>
	/// <returns></returns>
	virtual bool Start();
	/// <summary>
	/// ��ͣ����
	/// </summary>
	/// <returns></returns>
	virtual bool Stop();
	/// <summary>
	/// �Ƿ�������
	/// </summary>
	/// <returns></returns>
	virtual bool IsDownloading() const;
	/// <summary>
	/// ����guid
	/// </summary>
	void ReGenerateGUID();
	/// <summary>
	/// ������������json
	/// </summary>
	/// <param name="InFileName"></param>
	/// <returns></returns>
	bool SaveTaskToJsonFile(const FString& InFileName) const;


public:
	inline int32 GetGUID() const { return TaskInfo.GetGUID(); }
	inline FMTaskInformation GetTaskInformation() const { return TaskInfo; }
	inline EMTaskState GetState() const { return TaskState; }


public:
	// ����֪ͨ�����¼��Ļص��ĺ���ָ��
	TFunction<void(EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode)> PregressTaskFunc = [this](EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode)
		{
			// Ĭ��ָ��ĺ����߼�
			if (InEvent == EMTaskEvent::START_DOWNLOAD)
			{
				// __FUNCTION__ �� __LINE__ �� C++ �е�Ԥ����꣬���ڻ�ȡ��ǰ�������ڵĺ������ƺ��кš�
				UE_LOG(LogFileDownloader,
					Warning, 
					TEXT("%s  %d  Please use FileDownloadManager instead DownloadTask to download file.����ʹ��FileDownloadManager������DownloadTask�������ļ�����"),
					__FUNCTION__,
					__LINE__);
			}
		};


protected:
	/// <summary>
	/// ��ȡhttpͷ����Ϣ
	/// </summary>
	virtual void GetHead();
	/// <summary>
	/// ��ʼ�п�
	/// </summary>
	virtual void StartChunk();
	/// <summary>
	/// ��ȡ�������ļ�·��
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
	int32 ChunkSize = 2 * 1024 * 1024;	// 2MB��Ϊÿ��Ĵ�С
	TArray<uint8> DataBuffer;
	FString EncodedUrl;	// ���봦����url
	IFileHandle* TargetFilePtr = nullptr;
	FHttpRequestPtr RequestPtr = nullptr;
	int32 CurrentTypeCount = 0;	// ��ǰ��������Ĵ���
	int32 MaxTryCount = 5;	// ����������
	bool bNeedStop = false;

};
