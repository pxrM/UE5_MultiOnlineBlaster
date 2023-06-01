// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpDownloadSubTask.h"
#include "HttpModule.h"
#include "HttpMgrSubsystem.h"

FHttpDownloadSubTask::FHttpDownloadSubTask()
	:TaskID(INDEX_NONE)
	, bFinished(false)
	, StarPos(0)
	, Size(0)
	, bWaitRreponse(0)
{
}

FHttpDownloadSubTask::~FHttpDownloadSubTask()
{
	Stop();
}

TSharedPtr<IHttpRequest> FHttpDownloadSubTask::CreateRequest()
{
	RequestPtr = FHttpModule::Get().CreateRequest();
	RequestPtr->SetURL(URL);
	RequestPtr->SetVerb(TEXT("GET"));
	RequestPtr->SetHeader(TEXT("Range"), Range);	//下载该文件的一个范围
	bWaitRreponse = true;
	RequestTime = FApp::GetCurrentTime(); //当前运行时间

	return RequestPtr;
}

void FHttpDownloadSubTask::Stop()
{
	if (RequestPtr.IsValid())
	{
		RequestPtr->CancelRequest();
		RequestPtr = nullptr;
	}
}

void FHttpDownloadSubTask::SaveData()
{
	// hcf  =  http cache file
	FString SubTaskFileName = MD5Str + TEXT("_") + FString::FromInt(TaskID) + M_SUFFIX;
	FFileHelper::SaveArrayToFile(RawData, *(CurFilePath / SubTaskFileName));
}

void FHttpDownloadSubTask::LoadData()
{
	FString SubTaskFileName = MD5Str + TEXT("_") + FString::FromInt(TaskID) + M_SUFFIX;
	FFileHelper::LoadFileToArray(RawData, *(CurFilePath / SubTaskFileName));
}
