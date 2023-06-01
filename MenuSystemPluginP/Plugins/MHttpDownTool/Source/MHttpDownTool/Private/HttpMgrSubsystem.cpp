// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpMgrSubsystem.h"
#include "AsyncDownloadFile.h"
#include "HttpManagerSettings.h"

void UHttpMgrSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Init();
}

void UHttpMgrSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ClearDownloadHttp();
}

void UHttpMgrSubsystem::Tick(float DeltaTime)
{
	if (IsTickable())
	{
		for (int32 i = DownloadHttpFiles.Num() - 1; i >= 0; i--)
		{
			UAsyncDownloadFile* HttpFile = DownloadHttpFiles[i];
			if (HttpFile)
			{
				if (HttpFile->State == ETaskState::_Downloading)
				{
					HttpFile->CheckDeadTask(FApp::GetCurrentTime());
				}
			}
			else
			{
				DownloadHttpFiles.RemoveAt(i);
			}
		}
	}
}

bool UHttpMgrSubsystem::IsTickable() const
{
	return DownloadHttpFiles.Num() > 0;
}

UAsyncDownloadFile* UHttpMgrSubsystem::CreateDownTask(UObject* WorldContextObject, const FString& Url, const FString& FileName, const FString& Directory, bool bClearCache)
{
	//*(Directory / FileName) ��һ�������д������ C++ ��ͨ����Ϊ���������ַ������������Directory �� FileName �����ַ������͵ı�����/ ��ʾ������ƴ��������
	//���� / ����������ȼ��� * ��Ҫ�ߣ�������������ʽ�У�Directory / FileName �Ľ����һ�� const char* ���͵�ָ�룬ָ��ƴ����ɺ���ַ�����
	//Ȼ��ʹ�� * ����������������ָ�룬�õ�һ�� const char ���͵��ַ�������ַ���������
	UAsyncDownloadFile* HttpFile = NewObject<UAsyncDownloadFile>(WorldContextObject, *(Directory / FileName));
	if (HttpFile)
	{
		HttpFile->Url = Url;
		HttpFile->FileName = FileName;
		HttpFile->Directory = Directory;
		HttpFile->State = ETaskState::_Retry;
		HttpFile->HttpMgr = this;
		HttpFile->bClearCache = bClearCache;
		HttpFile->MD5Str = FMD5::HashAnsiString(*Url);
		DownloadHttpFiles.AddUnique(HttpFile);
	}
	return HttpFile;
}

void UHttpMgrSubsystem::Init()
{
	//GetDefault �� Unreal Engine �е�һ�����ڻ�ȡĬ�϶���ķ�����
	//�� Unreal Engine �У�ÿ�� UObject ��������඼������һ��Ĭ�϶���default object�������Ǹ���ľ�̬ʵ���������ṩȱʡֵ��Ĭ�����á�
	const UHttpManagerSettings* Config = GetDefault<UHttpManagerSettings>();
	MaxParallel = Config->MaxParallel;
	MaxTryCount = Config->MaxTryCount;
	RequestKBSize = Config->RequestKBSize;
	ReponseTimeout = Config->ResponseTimeout;
	CurFilePath = Config->CurFilePath;
}

void UHttpMgrSubsystem::ExecDownloadTasks()
{
	int32 NumActiveTask = 0;
	TArray<UAsyncDownloadFile*> ProbTasks;
	for (int32 i = DownloadHttpFiles.Num() - 1; i >= 0; i--)
	{
		UAsyncDownloadFile* HttpFile = DownloadHttpFiles[i];
		if (HttpFile)
		{
			ETaskState State = HttpFile->State;
			switch (State)
			{
			case ETaskState::_Ready:
				ProbTasks.Add(HttpFile);
				break;
			case ETaskState::_Downloading:
				NumActiveTask++;
				break;
			case ETaskState::_Success:
				DownloadHttpFiles.RemoveAt(i);
				break;
			case ETaskState::_Retry:
				if (HttpFile->TryCount <= MaxTryCount)
				{
					ProbTasks.Add(HttpFile);
				}
				break;
			case ETaskState::_Failed:
				break;
			default:
				break;
			}
		}
	}
	for (int32 i = 0; i < ProbTasks.Num(); i++)
	{
		UAsyncDownloadFile* HttpFile = DownloadHttpFiles[i];
		if (HttpFile && NumActiveTask < MaxParallel)
		{
			HttpFile->StartDownload();
		}
	}
}

void UHttpMgrSubsystem::NotifyFailed(UAsyncDownloadFile* HttpFile, bool bReconnection)
{
	if (bReconnection && HttpFile)
	{
		if (HttpFile->TryCount < MaxTryCount)
		{
			HttpFile->NotifyWait();
		}
		else
		{
			HttpFile->NotifyFailed();
		}
	}
	ExecDownloadTasks();
}

void UHttpMgrSubsystem::NotifyFinised(UAsyncDownloadFile* HttpFile)
{
	ExecDownloadTasks();
	//ClearDownloadHttp();
}

void UHttpMgrSubsystem::ClearDownloadHttp()
{
	for (int32 i = DownloadHttpFiles.Num() - 1; i >= 0; i--)
	{
		DownloadHttpFiles.RemoveAt(i);
		continue;
	}
}
