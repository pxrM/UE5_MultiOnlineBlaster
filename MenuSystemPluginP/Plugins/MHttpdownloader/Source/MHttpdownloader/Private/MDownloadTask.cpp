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
#include "Unix/UnixPlatformHttp.h"


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
		PregressTaskFunc(EMTaskEvent::ERROR_OCCUR, TaskInfo, -1);
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
	return false;
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
			UrlDirectory[i] = FPlatformHttp::UrlDecode(UrlDirectory[i]);
			// ������� URL
			Url += FString("/");
			Url += UrlDirectory[i];
		}
	}
}

void FMDownloadTask::GetHead()
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

	EncodedUrl = ProcessUrl();

	RequestPtr->SetVerb("HEAD");
	RequestPtr->SetURL(EncodedUrl);
	RequestPtr->OnProcessRequestComplete().BindRaw(this, &FMDownloadTask::OnGetHeadCompleted);
	RequestPtr->ProcessRequest();

	TaskState = EMTaskState::DOWNLOADING;
	PregressTaskFunc(EMTaskEvent::START_DOWNLOAD, TaskInfo, 0);
}

void FMDownloadTask::StartChunk()
{
}

FString FMDownloadTask::GetFullFileName() const
{
	return FString();
}

void FMDownloadTask::OnGetHeadCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWadSuccessful)
{
}

void FMDownloadTask::OnGetChunkCompleted(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWadSuccessful)
{
}

void FMDownloadTask::OnTaskCompleted()
{
}

void FMDownloadTask::OnWriteChunkEnd(int32 DataSize)
{
}
