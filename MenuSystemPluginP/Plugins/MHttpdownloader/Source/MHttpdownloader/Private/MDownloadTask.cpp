// Fill out your copyright notice in the Description page of Project Settings.
/*
	将变量定义放在.cpp文件中的好处是，可以避免对这些变量的多次定义。
	如果将变量定义放在.h文件中，当多个源文件都包含了该头文件时，就会出现变量重定义错误。
	因此，将变量定义放在.cpp文件中可以保证每个变量只被定义一次，避免了这种错误。

	同时，将变量定义放在.cpp文件中也可以防止变量被其他源文件意外修改，因为其他源文件无法直接访问该变量。
	如果需要在其他源文件中使用该变量，可以通过在.h文件中声明该变量并使用extern关键字来实现。

	总之，将变量定义放在.cpp文件中可以提高代码的可维护性和可靠性，避免了变量重定义错误和不必要的变量访问。
*/

#include "MDownloadTask.h"
#include "HAL/PlatformFileManager.h"	// 是一个单例对象，可以用于获取当前平台的文件管理器
#include "Misc/Paths.h"					// 提供了一些用于处理文件路径和目录路径的函数和数据结构
#include "Misc/FileHelper.h"			// 提供了一些用于文件操作的函数和工具
#include "HttpModule.h"					// 提供了访问 HTTP 服务的功能
#include "HttpManager.h"				// 提供了更高层次的 HTTP 请求封装和管理
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Unix/UnixPlatformHttp.h"


const FString TEMP_FILE_EXTERN = TEXT(".dlFile");	// 临时文件的扩展名
const FString TASK_JSON = TEXT(".task");			// 任务的JSON文件的扩展名
IPlatformFile* PlatformFilePtr = nullptr;			// 当前平台文件管理器的实例指针



FMDownloadTask::FMDownloadTask()
{
	if (PlatformFilePtr == nullptr)
	{
		// GetPlatformFile()方法来访问当前平台文件管理器的实例
		PlatformFilePtr = &FPlatformFileManager::Get().GetPlatformFile();
		/*
			GetLowerLevel()方法返回文件管理器的下一级别（低级别）的文件管理器实例
			UE中，文件管理器可以有多个级别，每个级别处理不同的文件操作。 
			更高级别的文件管理器通常会提供更高级别的文件操作接口，封装了更多的功能和抽象，以便开发人员可以更方便地进行文件操作。
			而更低级别的文件管理器则可能提供更底层、更直接的文件操作接口，适用于一些特定的需求或者平台限制。
			通过获取更低级别的文件管理器，开发人员可以绕过更高级别的抽象，并直接访问底层的文件操作接口，
			以实现一些需要更细粒度控制的操作或者针对特定平台的操作。这样可以在一定程度上增加代码的灵活性和可扩展性。
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
		// FPaths::GetCleanFilename，该函数会返回一个 FString 类型的干净文件名，即去除文件路径和扩展名的部分。
		// 例如，如果传入的文件路径为 "/Game/Textures/Texture.png"，则函数返回的干净文件名就是 "Texture"。
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
		// 已经在下载，忽略
		return false;
	}

	// 每次我们开始下载(包括从暂停恢复)，应该检查任务信息，因为在暂停期间可能会更改远程资源
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
			UE_LOG(LogFileDownloader, Warning, TEXT("Cannot create directory(创建目录失败) : %s"), *InDirectory);
		}
	}
}

FString FMDownloadTask::ProcessUrl()
{
	/*
		当Uri 路径中带中文字符时，需要进行百分比编码，否则无法正确解析Url路径和参数
		对 URL 中的编码字符进行解码。在 URL 中，一些特殊字符（例如空格）需要进行编码，才能被正确传输和解析
		例如，对于以下 URL：
			http://www.example.com/path/to%20file.html
			其中的 "%20" 表示空格字符，需要被解码还原。调用上述代码中的函数后，将得到以下结果：
			解码后为：http://www.example.com/path/to file.html
	*/
	FString Url = GetSourceUrl();
	static int32 URLTag = 8;
	int32 StartSlash = GetSourceUrl().Find(FString("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, URLTag);
	if (StartSlash > INDEX_NONE)
	{
		// 将 URL 分成左右两部分
		FString UrlLeft = Url.Left(StartSlash);
		FString UrlRight = Url.Mid(StartSlash);

		// 将右半部分的 URL 按斜杠分割成数组
		TArray<FString> UrlDirectory;
		UrlRight.ParseIntoArray(UrlDirectory, *FString("/"));

		Url = UrlLeft;  // eg = http://www.example.com
		for (int32 i = 0; i < UrlDirectory.Num(); ++i)
		{
			UrlDirectory[i] = FPlatformHttp::UrlDecode(UrlDirectory[i]);
			// 重新组合 URL
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
		// HttpManager负责管理请求的发送、接收和处理等操作，以及处理请求的回调和错误处理。
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
