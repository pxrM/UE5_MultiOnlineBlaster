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
	_Ready,			//׼��
	_Downloading,	//������
	_Success,		//�ɹ�
	_Retry,			//��������
	_Failed,		//ʧ��
};

/**
 *	UBlueprintAsyncActionBase  ��ͼ�첽�������
 */
UCLASS()
class MHTTPDOWNTOOL_API UAsyncDownloadFile : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

		friend class UHttpMgrSubsystem;

public:
	// FObjectInitializer�����ڴ��� UObject �����������͵�ʵ��ʱ��ʼ�������ԡ���ͨ�������ڹ��캯���г�ʼ����Ա����
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
	void StartDownload();	//��ʼ����
	void RequestFileSize();	 //�����ļ���С
	void HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);  //��������Ӧ֮�������д���Ļص�����

	void StartSubTaskDownload(TSharedRef<class FHttpDownloadSubTask> SubTaskRef);
	void HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 TaskId);
	void UpdateProgress();


private:
	// �㲥����ʧ��
	void NotifyFailed();
	// �㲥����ȴ�
	void NotifyWait();
	// ����������
	void AllocationSubTask();
	// ����������
	void CreateSubTask();
	// ����������
	void UpdateTask();
	// �ļ������ĳ�
	void OnFinished();
	// ��������ʱ��������
	void CheckDeadTask(double CurTime);
	// �����ļ�
	void SaveToFile();
	// ��������
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
		int32 WaitResponse;	 //�ȴ���Ӧ

	UPROPERTY()
		class UHttpMgrSubsystem* HttpMgr;

	TSharedPtr<IHttpRequest> FileSizeRequest;

	TArray<TSharedPtr<FHttpDownloadSubTask>> SubTaskPtrs;

};
