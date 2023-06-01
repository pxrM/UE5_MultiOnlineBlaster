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
		//��ʼִ������
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
	// ���� HTTP �������
	FileSizeRequest = FHttpModule::Get().CreateRequest();
	if (FileSizeRequest)
	{
		// �������� URL ������ʽ
		FileSizeRequest->SetURL(Url);
		FileSizeRequest->SetVerb(TEXT("GET"));
		// ��� Range ����ͷ������ǰ�����ֽڵ����� 
		// Range��HTTP����ͷ��һ����׼�ֶΣ�����ָ���ͻ�����Ҫ��ȡ�������ϵ�ĳ����Χ�ڵ����ݡ���ʵ��Ӧ���У�ͨ������֧�ֶϵ��������ܣ����������ش��ļ�ʱ���ֿ������λ�ȡ���ݡ�
		// ͨ������Range����ͷ����ȡ�ļ���ǰ�����ֽڣ�����������᷵�������ļ��Ĵ�С�����������ܻ�Ӱ���������ܣ�����ֻ��ȡ�ļ���ǰ�����ֽھͿ��Եõ��ļ��Ĵ�С��Ϣ�������˷Ѵ������Դ
		FileSizeRequest->SetHeader(TEXT("Range"), TEXT("bytes=0-1"));
		// �� HandleResponseFileSize ��Ϊ��Ӧ��ɺ�Ļص�����
		FileSizeRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncDownloadFile::HandleResponseFileSize);
		// ��������
		FileSizeRequest->ProcessRequest();
	}
}

void UAsyncDownloadFile::HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (bSucceeded && HttpResponse.IsValid())
	{
		// ��ȡ��Ӧ״̬��
		int32 ResultCode = HttpResponse->GetResponseCode();
		// �����Ӧ״̬��Ϊ 200 �� 206������� Content-Range �ֶ��Ի�ȡԶ���ļ���С
		if (EHttpResponseCodes::IsOk(ResultCode))
		{
			// ��ȡ��Ӧͷ�е� Content-Range �ֶΣ����ֶΰ������ļ��ܴ�С����Ϣ��
			FString RangeStr = HttpResponse->GetHeader("Content-Range");
			// ���� / �ָ� Content-Range �ֶΣ���ȡ�ļ��ܴ�С���ַ���
			FString RightStr, LeftStr;
			RangeStr.Split(TEXT("/"), &LeftStr, &RightStr);
			// ����ļ��ܴ�С���ַ����ǿգ�����ת��Ϊ int64 ���ʹ洢�� TotalFileSize ������
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
	const int32 TaskSize = HttpMgr->RequestKBSize * 1024; //1mb��С
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
