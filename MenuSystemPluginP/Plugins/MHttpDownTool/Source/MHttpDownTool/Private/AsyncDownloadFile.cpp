// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncDownloadFile.h"
#include "Kismet/GameplayStatics.h"
#include "HttpMgrSubsystem.h"
#include "HttpModule.h"
#include "HttpDownloadSubTask.h"


UAsyncDownloadFile::UAsyncDownloadFile(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	, TryCount(0)
	, Progerss(0)
	, CurFileSize(0)
	, TotalFileSize(0)
	, WaitResponse(0)
	, MaxTask(2)
{
}

UAsyncDownloadFile* UAsyncDownloadFile::DownLoadHttpFile(UObject* WorldContextObject, const FString& InUrl, const FString& InDirectory, const FString& InFileName, bool bClearCache)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	UHttpMgrSubsystem* MySubSystem = GameInstance->GetSubsystem<UHttpMgrSubsystem>();
	UAsyncDownloadFile* HttpDownTask = nullptr;
	if (MySubSystem)
	{
		HttpDownTask = MySubSystem->CreateDownTask(WorldContextObject, InUrl, InFileName, InDirectory, bClearCache);
		//开始执行任务
		MySubSystem->ExecDownloadTasks();
	}
	return HttpDownTask;
}

void UAsyncDownloadFile::StartDownload()
{
	State = ETaskState::_Downloading;
	if (Url.Len() > 0 && Url.Contains(TEXT("http")) && FileName.Len() > 0)
	{
		RequestFileSize();
	}
	else
	{
		FatalErr(TEXT("url and filename must set!!"));
	}
	TryCount++;
}

void UAsyncDownloadFile::RequestFileSize()
{
	// 创建 HTTP 请求对象
	FileSizeRequest = FHttpModule::Get().CreateRequest();
	if (FileSizeRequest)
	{
		// 设置请求 URL 和请求方式
		FileSizeRequest->SetURL(Url);
		FileSizeRequest->SetVerb(TEXT("GET"));
		// 添加 Range 请求头，请求前两个字节的内容 
		// Range是HTTP请求头的一个标准字段，用于指定客户端想要获取服务器上的某个范围内的数据。在实际应用中，通常用于支持断点续传功能，或者在下载大文件时，分块请求多次获取数据。
		// 通过设置Range请求头来获取文件的前两个字节，否则服务器会返回整个文件的大小，而这样可能会影响下载性能，所以只获取文件的前两个字节就可以得到文件的大小信息，避免浪费带宽和资源
		FileSizeRequest->SetHeader(TEXT("Range"), TEXT("bytes=0-1"));
		// 绑定 HandleResponseFileSize 作为响应完成后的回调函数
		FileSizeRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncDownloadFile::HandleResponseFileSize);
		// 发送请求
		FileSizeRequest->ProcessRequest();
	}
}

void UAsyncDownloadFile::HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (bSucceeded && HttpResponse.IsValid())
	{
		// 获取响应状态码
		int32 ResultCode = HttpResponse->GetResponseCode();
		// 如果响应状态码为 200 或 206，则解析 Content-Range 字段以获取远程文件大小
		if (EHttpResponseCodes::IsOk(ResultCode))
		{
			// 获取响应头中的 Content-Range 字段，该字段包含了文件总大小的信息。
			FString RangeStr = HttpResponse->GetHeader("Content-Range");
			// 按照 / 分割 Content-Range 字段，获取文件总大小的字符串
			FString RightStr, LeftStr;
			RangeStr.Split(TEXT("/"), &LeftStr, &RightStr);
			// 如果文件总大小的字符串非空，则将其转换为 int64 类型存储到 TotalFileSize 变量中
			if (RightStr.Len() > 0)
			{
				TotalFileSize = FCString::Atoi64(*RightStr);
			}
		}
	}
	else
	{
		FatalErr(TEXT("Response is null"));
	}

	FileSizeRequest.Reset();

	if (TotalFileSize <= 0)
	{
		HttpMgr->NotifyFailed(this, true);
	}
	else
	{
		AllocationSubTask();
	}
}

void UAsyncDownloadFile::StartSubTaskDownload(TSharedRef<class FHttpDownloadSubTask> SubTaskRef)
{
	auto RequestPtr = SubTaskRef->CreateRequest();
	RequestPtr->OnProcessRequestComplete().BindUObject(this, &UAsyncDownloadFile::HandleDownload, SubTaskRef->TaskID);
	RequestPtr->ProcessRequest();
}

void UAsyncDownloadFile::HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 TaskId)
{
	for (auto SubTemp : SubTaskPtrs)
	{
		if (SubTemp.IsValid() && SubTemp->TaskID == TaskId)
		{
			WaitResponse--;
			if (HttpResponse.IsValid() && bSucceeded)
			{
				int32 Code = HttpResponse->GetResponseCode();
				if (EHttpResponseCodes::IsOk(Code))
				{
					TSharedRef<FHttpDownloadSubTask> SubRef = SubTemp.ToSharedRef();
					SubRef->RawData = HttpResponse->GetContent();
					if (SubRef->RawData.Num() == SubRef->Size)
					{
						SubRef->bFinished = true;
						SubRef->SaveData();
						UpdateProgress();
					}
					else
					{
						FatalErr(TEXT("HandleDownload content size error."));
					}
					SubRef->bWaitRreponse = false;
					SubRef->RequestPtr.Reset();
					SubRef->RequestPtr = nullptr;
					if (State != ETaskState::_Success)
					{
						UpdateTask();
					}
				}
			}
			break;
		}
	}
}

void UAsyncDownloadFile::UpdateProgress()
{
	int64 SizeNum = 0;
	bool bSuccess = true;
	for (auto Item : SubTaskPtrs)
	{
		if (Item->bFinished)
		{
			SizeNum += Item->Size;
		}
		else
		{
			bSuccess = false;
		}
	}
	Progerss = float(SizeNum) / (float)TotalFileSize;
	if (OnProgress.IsBound())
	{
		OnProgress.Broadcast(FileName, Progerss);
	}
	if (bSuccess)
	{
		OnFinished();
	}
}

void UAsyncDownloadFile::NotifyFailed()
{
	State = ETaskState::_Failed;
	if (OnFail.IsBound())
	{
		OnFail.Broadcast(Directory / FileName, Progerss);
	}
}

void UAsyncDownloadFile::NotifyWait()
{
	State = ETaskState::_Ready;
	if (OnWait.IsBound())
	{
		OnWait.Broadcast(Directory / FileName, Progerss);
	}
}

void UAsyncDownloadFile::AllocationSubTask()
{
	CreateSubTask();
	UpdateTask();
}

void UAsyncDownloadFile::CreateSubTask()
{
	int32 StartSize = 0;
	SubTaskPtrs.Empty();
	int32 TaskId = 0;
	const int32 TaskSize = HttpMgr->RequestKBSize * 1024; //1mb大小
	TMap<int32, FString> SuccessSubTasks;
	FString CurFilePath = HttpMgr->CurFilePath / MD5Str;
	IPlatformFile& PlatFileModule = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatFileModule.DirectoryExists(*CurFilePath))
	{
		TArray<FString>ExistedFiles;
		SuccessSubTasks.Empty();
		PlatFileModule.FindFiles(ExistedFiles, *CurFilePath, M_SUFFIX);
		for (int32 i = 0; i < ExistedFiles.Num(); i++)
		{
			FString LStr, RStr;
			ExistedFiles[i].Split(TEXT("_"), &LStr, &RStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			FString LStr1, RStr2;
			RStr.Split(TEXT("."), &LStr1, &RStr2, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			SuccessSubTasks.Add(FCString::Atoi(*LStr1), LStr);
		}
	}
	while (StartSize < TotalFileSize)
	{
		TSharedPtr<FHttpDownloadSubTask> SubTask;
		int32 TempSize = 0;
		FString TempRange;
		SubTask = MakeShareable(new FHttpDownloadSubTask);
		SubTask->TaskID = TaskId;
		SubTask->URL = Url;
		SubTask->MD5Str = MD5Str;
		SubTask->StarPos = StartSize;
		SubTask->CurFilePath = CurFilePath;
		if (SuccessSubTasks.Contains(TaskId))
		{
			SubTask->bFinished = true;
			SubTask->bWaitRreponse = false;
			SubTask->LoadData();
		}
		if (TotalFileSize > (StartSize + TaskSize))
		{
			TempSize = TaskSize;
			TempRange = FString::Printf(TEXT("bytes=%d-%d"), StartSize, StartSize + TempSize - 1); //0-1023
		}
		else
		{
			TempSize = TotalFileSize - TaskSize;
			TempRange = FString::Printf(TEXT("bytes=%d-"), StartSize);
		}
		ensureMsgf(TempSize > 0, TEXT("SubTask Size Is Null"));
		if (SubTask.IsValid())
		{
			SubTask->Size = TempSize;
			SubTask->Range = TempRange;
			SubTaskPtrs.Add(SubTask);
		}
		StartSize += TempSize;
		++TaskId;
	}

	bool bSuccess = true;
	for (auto& Sub : SubTaskPtrs)
	{
		if (Sub.IsValid() && !Sub->bFinished)
		{
			bSuccess = false;
			break;
		}
	}
	if (bSuccess)
	{
		OnFinished();
	}
}

void UAsyncDownloadFile::UpdateTask()
{
	if (WaitResponse <= MaxTask)
	{
		for (int32 i = 0; i < SubTaskPtrs.Num(); i++)
		{
			if (SubTaskPtrs[i].IsValid())
			{
				TSharedRef<FHttpDownloadSubTask> SubTaskRef = SubTaskPtrs[i].ToSharedRef();
				if (!SubTaskRef->bFinished && !SubTaskRef->bWaitRreponse)
				{
					WaitResponse++;
					StartSubTaskDownload(SubTaskRef);
					break;
				}
			}
		}
	}
}

void UAsyncDownloadFile::OnFinished()
{
	SaveToFile();
	if (HttpMgr)
	{
		if (OnSuccess.IsBound())
		{
			OnSuccess.Broadcast(FileName, Progerss);
			HttpMgr->NotifyFinised(this);
		}
		if (bClearCache)
		{
			IPlatformFile& PlatFileModule = FPlatformFileManager::Get().GetPlatformFile();
			PlatFileModule.DeleteDirectoryRecursively(*(HttpMgr->CurFilePath / MD5Str));
		}
	}
}

void UAsyncDownloadFile::CheckDeadTask(double CurTime)
{
	if (WaitResponse <= MaxTask)
	{
		for (int32 i = 0; i < SubTaskPtrs.Num(); i++)
		{
			if (SubTaskPtrs[i].IsValid())
			{
				TSharedRef<FHttpDownloadSubTask> Task = SubTaskPtrs[i].ToSharedRef();
				if (!Task->bFinished && Task->bWaitRreponse)
				{
					float TimeOut = CurTime - Task->RequestTime;
					if (TimeOut > HttpMgr->ReponseTimeout)
					{
						WaitResponse++;
						StartSubTaskDownload(Task);
					}
				}
			}
		}
	}
}

void UAsyncDownloadFile::SaveToFile()
{
	FString FilePath = Directory / FileName;
	FArchive* Wirter = IFileManager::Get().CreateFileWriter(*FilePath);
	if (Wirter)
	{
		for (int32 i = 0; i < SubTaskPtrs.Num(); i++)
		{
			if (SubTaskPtrs[i].IsValid())
			{
				TSharedRef<FHttpDownloadSubTask> Task = SubTaskPtrs[i].ToSharedRef();
				Wirter->Serialize(Task->RawData.GetData(), Task->RawData.Num());
			}
		}
		Wirter->Close();
	}
}

void UAsyncDownloadFile::FatalErr(const FString& ErrMsg)
{
	State = ETaskState::_Failed;
	OnFail.Broadcast(ErrMsg, -1);
	if (HttpMgr)
	{
		HttpMgr->NotifyFailed(this, false);
	}
}
