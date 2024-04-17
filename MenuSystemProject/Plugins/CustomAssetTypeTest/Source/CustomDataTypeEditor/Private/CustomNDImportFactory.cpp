// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDImportFactory.h"
#include "CustomNormalDistribution.h"
#include "EditorFramework/AssetImportData.h"
// #include "Misc/ConfigCacheIni.h"

UCustomNDImportFactory::UCustomNDImportFactory()
{
	SupportedClass = UCustomNormalDistribution::StaticClass();

	// 表示在导入资源时不会创建新的资源对象。通常情况下，当导入一个新的资源时，会创建一个新的资源对象。但这里将其设置为 false，意味着导入的数据将会被用来填充现有的资源对象
	bCreateNew = false;
	// 添加了导入格式为 "cnd" 的支持。表示该工厂可以导入以 ".cnd" 扩展名结尾的文件，并且这些文件被视为 "Custom Normal Distribution" 类型的资源。
	Formats.Add(TEXT("cnd;Custom Normal Distribution"));
	// 表示这是一个用于编辑器导入的工厂。这意味着该工厂可以在编辑器中使用，用于导入资源。
	bEditorImport = true;
	// 表示支持文本导入。这意味着该工厂可以从text数据中创建资源，（另一种格式为二进制）。
	bText = true;
}

UObject* UCustomNDImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn)
{
	// 文本数据创建资源。它接收参数来描述要创建的资源的类、父级、名称、标志以及创建过程中的上下文信息等

	// 在导入资源之前，通过广播 OnAssetPreImport 事件来通知编辑器导入子系统，即将导入指定的资源类、父级、名称和类型。
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	if (InClass != UCustomNormalDistribution::StaticClass() || FCString::Stricmp(Type, TEXT("cnd")) != 0)
		return nullptr;

	UCustomNormalDistribution* Data = CastChecked<UCustomNormalDistribution>(NewObject<UCustomNormalDistribution>(InParent, InName, Flags));

	// 从文件获取值
	Data->Mean = FCString::Atof(*GetValueFromFile(Buffer, "[MySection]", "Mean"));
	Data->StandardDeviation = FCString::Atof(*GetValueFromFile(Buffer, "[MySection]", "StandardDeviation"));

	// 从文件获取值的另一种方式。特意将文件内容的书写格式与.ini相似，所以也可以借用.ini方式处理。
	// FConfigCacheIni Config(EConfigCacheType::Temporary);
	// Config.LoadFile(CurrentFilename);
	// Config.GetFloat(TEXT("MySection"), TEXT("Mean"), Data->Mean, CurrentFilename);
	// Config.GetFloat(TEXT("MySection"), TEXT("StandardDeviation"), Data->StandardDeviation, CurrentFilename);

	// 储存导入的路径
	Data->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, Data->GetPackage());
	// 通知编辑器导入子系统，表示导入操作已完成。
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, Data);

	return Data;
}

bool UCustomNDImportFactory::FactoryCanImport(const FString& Filename)
{
	// 用于检查工厂是否能够导入指定文件的资源。
	return FPaths::GetExtension(Filename).Equals(TEXT("cnd"));
}

FString UCustomNDImportFactory::GetValueFromFile(const TCHAR*& Buffer, FString SectionName, FString VarName)
{
	FString Str(Buffer);
	Str = Str.Replace(TEXT("\r"), TEXT(""));
	TArray<FString> Lines;
	Str.ParseIntoArray(Lines, TEXT("\n"), true);

	bool bInSection = false;

	for (FString Line : Lines)
	{
		if (Line == SectionName)
		{
			bInSection = true;
		}
		else if (Line.StartsWith("[") && Line.EndsWith("]"))
		{
			bInSection = false;
		}

		if (bInSection)
		{
			int32 Pos = Line.Find("=");
			if (Pos != INDEX_NONE)
			{
				FString Name = Line.Left(Pos);
				FString Value = Line.Mid(Pos + 1);

				if (Name == VarName)
				{
					return Value;
				}
			}
		}
	}

	return "";
}

bool UCustomNDImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	/*
		根据需要实现检查逻辑，并将重新导入的文件路径添加到 OutFilenames 中
		返回 true 表示可以重新导入，false 表示不可以重新导入
	*/

	UCustomNormalDistribution* Data = Cast<UCustomNormalDistribution>(Obj);
	if (Data)
	{
		OutFilenames.Add(UAssetImportData::ResolveImportFilename(Data->SourceFilePath, Data->GetPackage()));
		return true;
	}
	return false;
}

void UCustomNDImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	/*
		根据需要设置重新导入的文件路径
	*/

	UCustomNormalDistribution* Data = Cast<UCustomNormalDistribution>(Obj);
	if (Data && ensure(NewReimportPaths.Num() == 1))
	{
		Data->SourceFilePath = UAssetImportData::SanitizeImportFilename(NewReimportPaths[0], Data->GetPackage());
	}
}

EReimportResult::Type UCustomNDImportFactory::Reimport(UObject* Obj)
{
	/*
		用于触发资源的重新导入操作，它会调用 CanReimport 和 SetReimportPaths 函数来执行具体的重新导入逻辑。
		Obj：要重新导入的对象，通常是之前导入的资源。
		retuen: EReimportResult::Type：重新导入操作的结果类型，包括成功、失败等。
	*/

	UCustomNormalDistribution* Data = Cast<UCustomNormalDistribution>(Obj);
	if (!Data)
	{
		return EReimportResult::Failed;
	}
	
	const FString Filename = UAssetImportData::ResolveImportFilename(Data->SourceFilePath, Data->GetPackage());
	if (!FPaths::GetExtension(Filename).Equals(TEXT("cnd")))
	{
		return EReimportResult::Failed;
	}

	CurrentFilename = Filename;
	FString LoadedData;
	if (FFileHelper::LoadFileToString(LoadedData, *CurrentFilename))
	{
		const TCHAR* LoadedDataChar = *LoadedData;
		Data->Modify();
		Data->MarkPackageDirty();

		Data->Mean = FCString::Atof(*GetValueFromFile(LoadedDataChar, "[MySection]", "Mean"));
		Data->StandardDeviation =
			FCString::Atof(*GetValueFromFile(LoadedDataChar, "[MySection]", "StandardDeviation"));

		Data->SourceFilePath =
			UAssetImportData::SanitizeImportFilename(CurrentFilename, Data->GetPackage());
	}

	return EReimportResult::Succeeded;
}
