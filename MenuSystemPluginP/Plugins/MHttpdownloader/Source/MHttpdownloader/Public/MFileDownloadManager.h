// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "MTaskInformation.h"
#include "MFileDownloadManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMDLManagerDelegate, EMTaskEvent, InEvent, int32, InTaskID, int32, InHttpCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMOnAllTaskCompleted, int32, ErrorCount);


/**
 * 该类是插件的接口，尽量使用该类下载文件(c++ +蓝图)
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
	/// 按顺序启动所有任务的下载操作
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StartAllTask();

	/// <summary>
	/// 启动一个任务，只改变状态，如果当前工作到MaxDoingWorks，任务是等待
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StartTask(int32 InGuid);

	/// <summary>
	/// 停止所有任务，释放文件句柄并取消HTTP
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StopAll();

	/// <summary>
	/// 立即停止任务
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StopTask(int32 InGuid);

	/// <summary>
	/// 获取所有任务的总进度
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
	/// 停止并删除所有任务
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void ClearAllTask();

	/// <summary>
	/// 将任务信息保存到Json文件中，以便稍后加载任务。
	/// </summary>
	/// <param name="InGuid">id不能无效，标识一个任务</param>
	/// <param name="InFileName">InFileName找出目标json文件名，可以忽略这个参数</param>
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
	/// 添加一个新任务(存在的任务将被忽略，通过Guid检测)，首先不能为空!!
	/// </summary>
	/// <param name="InUrl"> InUrl不能为空! </param>
	/// <param name="InDirectory"> 忽略此参数(默认目录将使用../Content/FileDownload) </param>
	/// <param name="InFileName"> 忽略此参数(将使用默认文件名，从InUrl中剪切并复制名称) </param>
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
	/// tick间隔
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TickInterval = 0.1f;

	/// <summary>
	/// 同时进行的最大任务数量
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxParallelTask = 5;

	UPROPERTY(BlueprintAssignable)
	FMDLManagerDelegate OnDlManagerEvent;

	UPROPERTY(BlueprintAssignable)
	FMOnAllTaskCompleted OnAllTaskCompleted;


protected:
	TMap<int32, TSharedPtr<class FMDownloadTask>> TaskList;

	int32 CurrentDoingWorks = 0; // 当前进行的任务

	bool bStopAll = false;

	int32 ErrorCount = 0;

};
