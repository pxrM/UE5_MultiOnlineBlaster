// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "MTaskInformation.h"
#include "MFileDownloadManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMDLManagerDelegate, EMTaskEvent, InEvent, int32, InTaskID, int32, InHttpCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMOnAllTaskCompleted, int32, ErrorCount);


/**
 * �����ǲ���Ľӿڣ�����ʹ�ø��������ļ�(c++ +��ͼ)
 */
UCLASS()
class MHTTPDOWNLOADER_API UMFileDownloadManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;


public:
	/// <summary>
	/// ��˳������������������ز���
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StartAllTask();

	/// <summary>
	/// ����һ������ֻ�ı�״̬�������ǰ������MaxDoingWorks�������ǵȴ�
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StartTask(int32 InGuid);

	/// <summary>
	/// ֹͣ���������ͷ��ļ������ȡ��HTTP
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StopAll();

	/// <summary>
	/// ����ֹͣ����
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StopTask(int32 InGuid);

	/// <summary>
	/// ��ȡ����������ܽ���
	/// </summary>
	/// <returns> tasks percent [0, 100] </returns>
	UFUNCTION(BlueprintCallable)
	int32 GetTotalPercent() const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="OutCurrentSize"></param>
	/// <param name="OutTotalSize"></param>
	UFUNCTION(BlueprintCallable)
	void GetByteSize(int64& OutCurrentSize, int64& OutTotalSize) const;

	/// <summary>
	/// ֹͣ��ɾ����������
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void ClearAllTask();

	/// <summary>
	/// ��������Ϣ���浽Json�ļ��У��Ա��Ժ��������
	/// </summary>
	/// <param name="InGuid">id������Ч����ʶһ������</param>
	/// <param name="InFileName">InFileName�ҳ�Ŀ��json�ļ��������Ժ����������</param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	bool SaveTaskToJsonFile(int32 InGuid, const FString& InFileName);

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	TArray<FMTaskInformation> GetAllTaskInformation() const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="InGuid"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	FMTaskInformation GetTaskInfo(int32 InGuid) const;

	/// <summary>
	/// ���һ��������(���ڵ����񽫱����ԣ�ͨ��Guid���)�����Ȳ���Ϊ��!!
	/// </summary>
	/// <param name="InUrl"> InUrl����Ϊ��! </param>
	/// <param name="InDirectory"> ���Դ˲���(Ĭ��Ŀ¼��ʹ��../Content/FileDownload) </param>
	/// <param name="InFileName"> ���Դ˲���(��ʹ��Ĭ���ļ�������InUrl�м��в���������) </param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	int32 AddTaskByUrl(const FString& InUrl, const FString& InDirectory = TEXT(""), const FString& InFileName = TEXT(""));

	/// <summary>
	/// 
	/// </summary>
	/// <param name="InGuid"></param>
	/// <param name="InTotalSize"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable)
	bool SetTotalSizeByGuid(int32 InGuid, int32 InTotalSize);


protected:
	void OnProcessTaskEvent(EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode);

	int32 FindTaskToDo() const;


public:
	/// <summary>
	/// tick���
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TickInterval = 0.1f;

	/// <summary>
	/// ͬʱ���е������������
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxParallelTask = 5;

	UPROPERTY(BlueprintAssignable)
	FMDLManagerDelegate OnDlManagerEvent;

	UPROPERTY(BlueprintAssignable)
	FMOnAllTaskCompleted OnAllTaskCompleted;


protected:
	TMap<int32, TSharedPtr<class FMDownloadTask>> TaskList;

	int32 CurrentDoingWorks = 0; // ��ǰ���е�����

	bool bStopAll = false;

	int32 ErrorCount = 0;

};
