// Fill out your copyright notice in the Description page of Project Settings.


#include "MFileDownloadManager.h"
#include "MDownloadTask.h"
#include "Misc/Paths.h"

void UMFileDownloadManager::BeginDestroy()
{
	StopAll();
	Super::BeginDestroy();
}

void UMFileDownloadManager::Tick(float DeltaTime)
{
	if (bStopAll)
	{
		return;
	}

	static float TimeCount = 0.f;
	TimeCount += DeltaTime;
	if (TimeCount >= TickInterval)
	{
		TimeCount = 0.f;
		if (CurrentDoingWorks < MaxParallelTask && TaskList.Num())
		{
			int32 Idx = FindTaskToDo();
			if (Idx > INDEX_NONE)
			{
				TaskList[Idx]->Start();
				++CurrentDoingWorks;
			}
		}
	}
}

TStatId UMFileDownloadManager::GetStatId() const
{
	return TStatId();
}

void UMFileDownloadManager::StartAllTask()
{
	bStopAll = false;
	for (auto It : TaskList)
	{
		It.Value->SetNeedStop(false);
	}
}

void UMFileDownloadManager::StartTask(int32 InGuid)
{
	if (TaskList.Contains(InGuid))
	{
		TaskList[InGuid]->SetNeedStop(false);
		bStopAll = false;
	}
}

void UMFileDownloadManager::StopAll()
{
	for (auto It : TaskList)
	{
		It.Value->Stop();
	}
	bStopAll = true;
	CurrentDoingWorks = 0;
}

void UMFileDownloadManager::StopTask(int32 InGuid)
{
	if (TaskList.Contains(InGuid))
	{
		if (TaskList[InGuid]->GetState() == EMTaskState::DOWNLOADING)
		{
			TaskList[InGuid]->Stop();
			--CurrentDoingWorks;
		}
	}
}

int32 UMFileDownloadManager::GetTotalPercent() const
{
	int64 CurrentSize = 0;
	int64 TotalSize = 0;

	for (const auto It : TaskList)
	{
		CurrentSize += It.Value->GetCurrentSize();
		TotalSize += It.Value->GetTotalSize();
	}

	if (TotalSize < 1)
	{
		return 0;
	}

	return (float)(CurrentSize) / TotalSize * 100.f;
}

void UMFileDownloadManager::GetByteSize(int64& OutCurrentSize, int64& OutTotalSize) const
{
	OutCurrentSize = 0;
	OutTotalSize = 0;

	for (const auto It : TaskList)
	{
		OutCurrentSize += It.Value->GetCurrentSize();
		OutTotalSize += It.Value->GetTotalSize();
	}
}

void UMFileDownloadManager::ClearAllTask()
{
	StopAll();
	TaskList.Reset();
	ErrorCount = 0;
	CurrentDoingWorks = 0;
}

bool UMFileDownloadManager::SaveTaskToJsonFile(int32 InGuid, const FString& InFileName)
{
	if (TaskList.Contains(InGuid))
	{
		return TaskList[InGuid]->SaveTaskToJsonFile(InFileName);

	}
	return false;
}

TArray<FMTaskInformation> UMFileDownloadManager::GetAllTaskInformation() const
{
	TArray<FMTaskInformation> Ret;
	for (auto It : TaskList)
	{
		Ret.Add(It.Value->GetTaskInformation());
	}

	return Ret;
}

FMTaskInformation UMFileDownloadManager::GetTaskInfo(int32 InGuid) const
{
	FMTaskInformation Ret;

	if (TaskList.Contains(InGuid))
	{
		Ret = TaskList[InGuid]->GetTaskInformation();
	}

	return Ret;
}

int32 UMFileDownloadManager::AddTaskByUrl(const FString& InUrl, const FString& InDirectory, const FString& InFileName)
{
	FString TmpDir = InDirectory;
	if (TmpDir.IsEmpty())
	{
		TmpDir = FPaths::ProjectSavedDir();
	}

	for (auto It : TaskList)
	{
		if (It.Value->GetSourceUrl() == InUrl)
		{
			return It.Key;
		}
	}

	TSharedPtr<FMDownloadTask> TaskPtr = MakeShareable(new FMDownloadTask(InUrl, TmpDir, InFileName));
	if (!TaskPtr)
	{
		return INDEX_NONE;
	}

	TaskPtr->ProcessTaskFunc = [this](EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHpptCode)
		{
			if (this)
			{
				this->OnProcessTaskEvent(InEvent, InInfo, InHpptCode);
			}
		};

	TaskList.Add(TaskPtr->GetGUID(), TaskPtr);
	return TaskPtr->GetGUID();
}

bool UMFileDownloadManager::SetTotalSizeByGuid(int32 InGuid, int32 InTotalSize)
{
	if (TaskList.Contains(InGuid) && TaskList[InGuid]->GetTotalSize() < 1)
	{
		TaskList[InGuid]->SetTotalSize(InTotalSize);
		return true;
	}

	return false;
}

void UMFileDownloadManager::OnProcessTaskEvent(EMTaskEvent InEvent, const FMTaskInformation& InInfo, int32 InHttpCode)
{
	OnDlManagerEvent.Broadcast(InEvent, InInfo.GetGUID(), InHttpCode);

	if (InEvent >= EMTaskEvent::DOWNLOAD_COMPLETED)
	{
		if (CurrentDoingWorks > 0)
		{
			--CurrentDoingWorks;
		}

		if (InEvent == EMTaskEvent::ERROR_OCCUR)
		{
			++ErrorCount;
		}

		if (CurrentDoingWorks < 1)
		{
			OnAllTaskCompleted.Broadcast(ErrorCount);
			ErrorCount = 0;
		}
	}
}

int32 UMFileDownloadManager::FindTaskToDo() const
{
	for (auto It : TaskList)
	{
		if (It.Value->GetState() == EMTaskState::WAIT && It.Value->GetNeedStop() == false)
		{
			return It.Key;
		}
	}
	return INDEX_NONE;
}
