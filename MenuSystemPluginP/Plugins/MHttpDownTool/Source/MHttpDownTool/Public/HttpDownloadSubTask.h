// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * 
 */
class MHTTPDOWNTOOL_API FHttpDownloadSubTask : public TSharedFromThis<FHttpDownloadSubTask>
{
public:
	FHttpDownloadSubTask();
	~FHttpDownloadSubTask();

public:
	TSharedPtr<IHttpRequest> CreateRequest();
	void Stop();
	void SaveData();
	void LoadData();

public:
	int32 TaskID;
	FString URL;
	FString MD5Str;
	FString CurFilePath;
	FString Range;	//下载范围
	bool bFinished;	//下载是否完成
	bool bWaitRreponse; //是否需要等待
	int32 StarPos;  //文件下载的字节位置
	int32 Size; //子任务的字节大小
	TArray<uint8> RawData; //下载后的数据
	double RequestTime; //请求时间
	TSharedPtr<IHttpRequest> RequestPtr; //请求指针
};
