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
	//*(Directory / FileName) 是一种特殊的写法，在 C++ 中通常称为“解引用字符串”。在这里，Directory 和 FileName 都是字符串类型的变量，/ 表示将它们拼接起来。
	//由于 / 运算符的优先级比 * 还要高，所以在这个表达式中，Directory / FileName 的结果是一个 const char* 类型的指针，指向拼接完成后的字符串。
	//然后，使用 * 运算符来解引用这个指针，得到一个 const char 类型的字符数组或字符串常量。
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
	//GetDefault 是 Unreal Engine 中的一种用于获取默认对象的方法。
	//在 Unreal Engine 中，每个 UObject 类的派生类都可以有一个默认对象（default object），它是该类的静态实例，用于提供缺省值和默认设置。
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
