// Fill out your copyright notice in the Description page of Project Settings.
/*
	�������������.cpp�ļ��еĺô��ǣ����Ա������Щ�����Ķ�ζ��塣
	����������������.h�ļ��У������Դ�ļ��������˸�ͷ�ļ�ʱ���ͻ���ֱ����ض������
	��ˣ��������������.cpp�ļ��п��Ա�֤ÿ������ֻ������һ�Σ����������ִ���

	ͬʱ���������������.cpp�ļ���Ҳ���Է�ֹ����������Դ�ļ������޸ģ���Ϊ����Դ�ļ��޷�ֱ�ӷ��ʸñ�����
	�����Ҫ������Դ�ļ���ʹ�øñ���������ͨ����.h�ļ��������ñ�����ʹ��extern�ؼ�����ʵ�֡�

	��֮���������������.cpp�ļ��п�����ߴ���Ŀ�ά���ԺͿɿ��ԣ������˱����ض������Ͳ���Ҫ�ı������ʡ�
*/

#include "MDownloadTask.h"
#include "HAL/PlatformFileManager.h"	// ��һ���������󣬿������ڻ�ȡ��ǰƽ̨���ļ�������
#include "Misc/Paths.h"					// �ṩ��һЩ���ڴ����ļ�·����Ŀ¼·���ĺ��������ݽṹ
#include "Misc/FileHelper.h"			// �ṩ��һЩ�����ļ������ĺ����͹���
#include "HttpModule.h"					// �ṩ�˷��� HTTP ����Ĺ���
#include "HttpManager.h"				// �ṩ�˸��߲�ε� HTTP �����װ�͹���
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Async/Async.h"


const FString TEMP_FILE_EXTERN = TEXT(".dlFile");	// ��ʱ�ļ�����չ��
const FString TASK_JSON = TEXT(".task");			// �����JSON�ļ�����չ��
IPlatformFile* PlatformFilePtr = nullptr;			// ��ǰƽ̨�ļ���������ʵ��ָ��



FMDownloadTask::FMDownloadTask()
{
	if (PlatformFilePtr == nullptr)
	{
		// GetPlatformFile()���������ʵ�ǰƽ̨�ļ���������ʵ��
		PlatformFilePtr = &FPlatformFileManager::Get().GetPlatformFile();
		/*
			GetLowerLevel()���������ļ�����������һ���𣨵ͼ��𣩵��ļ�������ʵ��
			UE�У��ļ������������ж������ÿ��������ͬ���ļ�������
			���߼�����ļ�������ͨ�����ṩ���߼�����ļ������ӿڣ���װ�˸���Ĺ��ܺͳ����Ա㿪����Ա���Ը�����ؽ����ļ�������
			�����ͼ�����ļ�������������ṩ���ײ㡢��ֱ�ӵ��ļ������ӿڣ�������һЩ�ض����������ƽ̨���ơ�
			ͨ����ȡ���ͼ�����ļ���������������Ա�����ƹ����߼���ĳ��󣬲�ֱ�ӷ��ʵײ���ļ������ӿڣ�
			��ʵ��һЩ��Ҫ��ϸ���ȿ��ƵĲ�����������ض�ƽ̨�Ĳ���������������һ���̶������Ӵ��������ԺͿ���չ�ԡ�
		*/
		if (PlatformFilePtr->GetLowerLevel())
		{
			PlatformFilePtr = PlatformFilePtr->GetLowerLevel();
		}
	}
	ReGenerateGUID();
}

FMDownloadTask::FMDownloadTask(const FString& InUrl, const FString& InDirectory, const FString& InFileName)
	:FMDownloadTask()
{
	CreateDirectory(InDirectory);
	SetDestDirectory(InDirectory);
	SetSourceUrl(InUrl);
	SetFileName(InFileName);
}

FMDownloadTask::FMDownloadTask(const FMTaskInformation& InTaskInfo)
	:FMDownloadTask()
{
	CreateDirectory(InTaskInfo.DestDirectory);
	TaskInfo = InTaskInfo;
}

FMDownloadTask::~FMDownloadTask()
{
	Stop();
}

void FMDownloadTask::SetFileName(const FString& InFileName)
{
	TaskInfo.FileName = InFileName;
}

const FString& FMDownloadTask::GetFileName() const
{
	return TaskInfo.FileName;
}

void FMDownloadTask::SetDestDirectory(const FString& InDestDirectory)
{
	TaskInfo.DestDirectory = InDestDirectory;
}

const FString& FMDownloadTask::GetDestDirectory() const
{
	return TaskInfo.DestDirectory;
}

void FMDownloadTask::SetSourceUrl(const FString& InSourceUrl)
{
	TaskInfo.SourceUrl = InSourceUrl;
}

const FString& FMDownloadTask::GetSourceUrl() const
{
	return TaskInfo.SourceUrl;
}

void FMDownloadTask::SetETag(const FString& InETag)
{
	TaskInfo.ETag = InETag;
}

const FString& FMDownloadTask::GetETag() const
{
	return TaskInfo.ETag;
}

void FMDownloadTask::SetCurrentSize(int32 InCurrentSize)
{
	TaskInfo.CurrentSize = InCurrentSize;
}

int32 FMDownloadTask::GetCurrentSize() const
{
	return TaskInfo.CurrentSize;
}

void FMDownloadTask::SetTotalSize(int32 InTotalSize)
{
	TaskInfo.TotalSize = InTotalSize;
}

int32 FMDownloadTask::GetTotalSize() const
{
	return TaskInfo.TotalSize;
}

void FMDownloadTask::SetNeedStop(const bool InNeedStop)
{
	bNeedStop = InNeedStop;
}

const bool FMDownloadTask::GetNeedStop() const
{
	return bNeedStop;
}

int32 FMDownloadTask::GetPercentage() const
{
	int32 Total = GetTotalSize();
	if (Total < 1)
	{
		return 0;
	}
	else
	{
		float Pregress = GetCurrentSize() / Total;
		return Pregress;
	}
}

bool FMDownloadTask::Start()
{
	SetNeedStop(false);

	if (GetFileName().IsEmpty())
	{
		// FPaths::GetCleanFilename���ú����᷵��һ�� FString ���͵ĸɾ��ļ�������ȥ���ļ�·������չ���Ĳ��֡�
		// ���磬���������ļ�·��Ϊ "/Game/Textures/Texture.png"���������صĸɾ��ļ������� "Texture"��
		SetFileName(FPaths::GetCleanFilename(GetSourceUrl()));
	}

	if (GetSourceUrl().IsEmpty() || GetFileName().IsEmpty())
	{
		TaskState = EMTaskState::ERROR;
		ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, -1);
		return false;
	}

	if (IsDownloading())
	{
		// �Ѿ������أ�����
		return false;
	}

	// ÿ�����ǿ�ʼ����(��������ͣ�ָ�)��Ӧ�ü��������Ϣ����Ϊ����ͣ�ڼ���ܻ����Զ����Դ
	GetHead();

	return true;
}

bool FMDownloadTask::Stop()
{
	return false;
}

bool FMDownloadTask::IsDownloading() const
{
	return TaskState == EMTaskState::DOWNLOADING;
}

void FMDownloadTask::ReGenerateGUID()
{
	static int32 GUIDTemp = 0;
	++GUIDTemp;
	TaskInfo.GUID = GUIDTemp;
}

bool FMDownloadTask::SaveTaskToJsonFile(const FString& InFileName) const
{
	FString TmpName = InFileName;
	if (TmpName.IsEmpty() == true)
	{
		TmpName = GetFullFileName() + TASK_JSON;
	}

	FString OutStr;
	TaskInfo.SerializeToJsonString(OutStr);

	return FFileHelper::SaveStringToFile(OutStr, *TmpName);
}

void FMDownloadTask::CreateDirectory(const FString& InDirectory)
{
	if (PlatformFilePtr->DirectoryExists(*InDirectory) == false)
	{
		if (PlatformFilePtr->CreateDirectoryTree(*InDirectory))
		{
			UE_LOG(LogFileDownloader, Warning, TEXT("Cannot create directory(����Ŀ¼ʧ��) : %s"), *InDirectory);
		}
	}
}

FString FMDownloadTask::ProcessUrl()
{
	/*
		��Uri ·���д������ַ�ʱ����Ҫ���аٷֱȱ��룬�����޷���ȷ����Url·���Ͳ���
		�� URL �еı����ַ����н��롣�� URL �У�һЩ�����ַ�������ո���Ҫ���б��룬���ܱ���ȷ����ͽ���
		���磬�������� URL��
			http://www.example.com/path/to%20file.html
			���е� "%20" ��ʾ�ո��ַ�����Ҫ�����뻹ԭ���������������еĺ����󣬽��õ����½����
			�����Ϊ��http://www.example.com/path/to file.html
	*/
	FString Url = GetSourceUrl();
	static int32 URLTag = 8;
	int32 StartSlash = GetSourceUrl().Find(FString("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, URLTag);
	if (StartSlash > INDEX_NONE)
	{
		// �� URL �ֳ�����������
		FString UrlLeft = Url.Left(StartSlash);
		FString UrlRight = Url.Mid(StartSlash);

		// ���Ұ벿�ֵ� URL ��б�ָܷ������
		TArray<FString> UrlDirectory;
		UrlRight.ParseIntoArray(UrlDirectory, *FString("/"));

		Url = UrlLeft;  // eg = http://www.example.com
		for (int32 i = 0; i < UrlDirectory.Num(); ++i)
		{
			UrlDirectory[i] = FGenericPlatformHttp::UrlDecode(UrlDirectory[i]);
			// ������� URL
			Url += FString("/");
			Url += UrlDirectory[i];
		}
	}
	return Url;
}

void FMDownloadTask::InitializeRequestPtr()
{
#if PLATFORM_IOS
	RequestPtr = FHttpModule::Get().CreateRequest();
#else
	if (RequestPtr.IsValid() == false)
	{
		RequestPtr = FHttpModule::Get().CreateRequest();
		// HttpManager�����������ķ��͡����պʹ���Ȳ������Լ���������Ļص��ʹ�����
		FHttpModule::Get().GetHttpManager().AddRequest(RequestPtr.ToSharedRef());
	}
#endif // PLATFORM_IOS
}

void FMDownloadTask::GetHead()
{
	InitializeRequestPtr();

	EncodedUrl = ProcessUrl();

	RequestPtr->SetVerb("HEAD");
	RequestPtr->SetURL(EncodedUrl);
	RequestPtr->OnProcessRequestComplete().BindRaw(this, &FMDownloadTask::OnGetHeadCompleted);
	RequestPtr->ProcessRequest();

	TaskState = EMTaskState::DOWNLOADING;
	ProcessTaskFunc(EMTaskEvent::START_DOWNLOAD, TaskInfo, 0);
}

void FMDownloadTask::StartChunk()
{
	InitializeRequestPtr();

	RequestPtr->SetVerb("GET");
	RequestPtr->SetURL(EncodedUrl);

	// ��ǰ������ط�Χ
	int32 StartPostion = GetCurrentSize();
	int32 EndPosition = StartPostion + ChunkSize - 1;

	if (EndPosition >= GetTotalSize())
	{
		EndPosition = GetTotalSize() - 1;
	}

	if (StartPostion >= EndPosition)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("Error! StartPostion >= EndPosition"));
		return;
	}

	FString RangeStr = FString::Printf(TEXT("bytes=%d-%d"), StartPostion, EndPosition);
	RequestPtr->SetHeader(TEXT("Range"), RangeStr);

	RequestPtr->OnProcessRequestComplete().BindRaw(this, &FMDownloadTask::OnGetChunkCompleted);
	RequestPtr->ProcessRequest();
}

FString FMDownloadTask::GetFullFileName() const
{
	return  FPaths::Combine(GetDestDirectory(), GetFileName());
}

void FMDownloadTask::OnGetHeadCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWasSuccessful)
{
	if (InResponse.IsValid() == false || bWasSuccessful == false)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("%s:%d"), UTF8_TO_TCHAR(__FUNCTION__), __LINE__);
		if (CurrentTryCount >= MaxTryCount)
		{
			TaskState = EMTaskState::ERROR;
			ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, InResponse.IsValid() ? InResponse->GetResponseCode() : 0);
		}
		else
		{
			TaskState = EMTaskState::WAIT;
			++CurrentTryCount;
			Start();
		}
		return;
	}

	int32 RetutnCode = InResponse->GetResponseCode();

	if (EHttpResponseCodes::IsOk(RetutnCode) == false)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("Http return code error : %d"), RetutnCode);
		if (TargetFilePtr)
		{
			delete TargetFilePtr;
			TargetFilePtr = nullptr;
		}
		TaskState = EMTaskState::ERROR;
		ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, RetutnCode);
		return;
	}

	if (RetutnCode == EHttpResponseCodes::Ok)
	{
		SetTotalSize(InResponse->GetContentLength());
	}

	if (TargetFilePtr)
	{
		delete TargetFilePtr;
		TargetFilePtr = nullptr;
	}
	// ���·����ڴ沢����ָ��һ���µ��ļ�
	TargetFilePtr = PlatformFilePtr->OpenWrite(*FString(GetFullFileName() + TEMP_FILE_EXTERN), true);

	if (TargetFilePtr == nullptr)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("create temp file error !"));
		ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, RetutnCode);
		TaskState = EMTaskState::ERROR;
		return;
	}
	else
	{
		SetCurrentSize(TargetFilePtr->Size());
	}

	FString TempJsonStr;
	FMTaskInformation ExistTaskInfo;
	if (FFileHelper::LoadFileToString(TempJsonStr, *FString(GetFullFileName() + TASK_JSON)))
	{
		ExistTaskInfo.DeserializeFromJsonString(TempJsonStr);
	}

	// Զ���ļ��Ѹ��£���Ҫ��������
	FString NewETag = InResponse->GetHeader("ETag");
	SetETag(NewETag);
	if (NewETag.IsEmpty() || NewETag != ExistTaskInfo.ETag)
	{
		SetCurrentSize(0);
	}

	// ���Ŀ���ļ��Ѿ����ڣ�����ɴ�����
	bool bExist = PlatformFilePtr->FileExists(*GetFullFileName());
	if (bExist && !NewETag.IsEmpty() && NewETag == ExistTaskInfo.ETag)
	{
		delete TargetFilePtr;
		TargetFilePtr = nullptr;
		PlatformFilePtr->DeleteFile(*FString(GetFullFileName() + TEMP_FILE_EXTERN));

		SetCurrentSize(GetTotalSize());

		OnTaskCompleted();
		return;
	}

	// ��������Ϣ���浽����
	SaveTaskToJsonFile(FString(""));

	StartChunk();
}

void FMDownloadTask::OnGetChunkCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWasSuccessful)
{
	if (bNeedStop)
	{
		TaskState = EMTaskState::WAIT;
		ProcessTaskFunc(EMTaskEvent::STOP, TaskInfo, InResponse->GetResponseCode());

		if (TargetFilePtr)
		{
			delete TargetFilePtr;
			TargetFilePtr = nullptr;
		}
		return;
	}

	if (InResponse.IsValid() == false || bWasSuccessful == false)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("%s:%d"), UTF8_TO_TCHAR(__FUNCTION__), __LINE__);
		if (CurrentTryCount >= MaxTryCount)
		{
			TaskState = EMTaskState::ERROR;
			ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, InResponse.IsValid() ? InResponse->GetResponseCode() : 0);
		}
		else
		{
			TaskState = EMTaskState::WAIT;
			++CurrentTryCount;
			Start();
		}
		return;
	}

	int32 RetutnCode = InResponse->GetResponseCode();

	if (EHttpResponseCodes::IsOk(RetutnCode) == false)
	{
		UE_LOG(LogFileDownloader, Warning, TEXT("%s, Return code error: %d"), *GetSourceUrl(), InResponse->GetResponseCode());
		if (TargetFilePtr)
		{
			delete TargetFilePtr;
			TargetFilePtr = nullptr;
		}
		TaskState = EMTaskState::ERROR;
		ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, RetutnCode);
		return;
	}

	DataBuffer = InResponse->GetContent();

	// �첽д��黺�������ļ���AsyncExecution::ThreadPool��ʾ�������ύ��ȫ�ֶ����̳߳���ִ�С�
	Async(EAsyncExecution::ThreadPool, [this]()->int32
		{
			if (this->TargetFilePtr)
			{
				// ���ļ�ָ�붨λ����ǰλ�ã����Ѿ�д�������֮���λ�ã�
				this->TargetFilePtr->Seek(this->GetCurrentSize());
				bool bWriteRet = this->TargetFilePtr->Write(DataBuffer.GetData(), DataBuffer.Num());
				if (bWriteRet)
				{
					// ˢ���ļ�������������
					this->TargetFilePtr->Flush();
					// ������Ϸ�߳�
					FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
						{
							this->OnWriteChunkEnd(this->DataBuffer.Num());
						}, TStatId(), nullptr, ENamedThreads::GameThread);
				}
				else
				{
					// ������Ϸ�߳�
					FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
						{
							UE_LOG(LogFileDownloader, Warning, TEXT("%s, %d, Async write file error !"), __FUNCTION__, __LINE__);
							this->TaskState = EMTaskState::ERROR;
							this->ProcessTaskFunc(EMTaskEvent::ERROR_OCCUR, this->TaskInfo, -1);
						}, TStatId(), nullptr, ENamedThreads::GameThread);
				}
				return 0;
			}
			else
			{
				return -1;
			}
		}); 
}

void FMDownloadTask::OnTaskCompleted()
{

}

void FMDownloadTask::OnWriteChunkEnd(int32 DataSize)
{
	if (GetState() != EMTaskState::DOWNLOADING)
	{
		return;
	}

	//update progress
	SetCurrentSize(GetCurrentSize() + DataSize);

	if (GetCurrentSize() < GetTotalSize())
	{
		ProcessTaskFunc(EMTaskEvent::DOWNLOAD_UPDATE, TaskInfo, 0);
		//download next chunk
		StartChunk();
	}
	else
	{
		//task have completed.
		OnTaskCompleted();
	}
}
