// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDImportFactory.h"
#include "CustomNormalDistribution.h"
#include "EditorFramework/AssetImportData.h"
// #include "Misc/ConfigCacheIni.h"

UCustomNDImportFactory::UCustomNDImportFactory()
{
	SupportedClass = UCustomNormalDistribution::StaticClass();

	// ��ʾ�ڵ�����Դʱ���ᴴ���µ���Դ����ͨ������£�������һ���µ���Դʱ���ᴴ��һ���µ���Դ���󡣵����ｫ������Ϊ false����ζ�ŵ�������ݽ��ᱻ����������е���Դ����
	bCreateNew = false;
	// ����˵����ʽΪ "cnd" ��֧�֡���ʾ�ù������Ե����� ".cnd" ��չ����β���ļ���������Щ�ļ�����Ϊ "Custom Normal Distribution" ���͵���Դ��
	Formats.Add(TEXT("cnd;Custom Normal Distribution"));
	// ��ʾ����һ�����ڱ༭������Ĺ���������ζ�Ÿù��������ڱ༭����ʹ�ã����ڵ�����Դ��
	bEditorImport = true;
	// ��ʾ֧���ı����롣����ζ�Ÿù������Դ�text�����д�����Դ������һ�ָ�ʽΪ�����ƣ���
	bText = true;
}

UObject* UCustomNDImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn)
{
	// �ı����ݴ�����Դ�������ղ���������Ҫ��������Դ���ࡢ���������ơ���־�Լ����������е���������Ϣ��

	// �ڵ�����Դ֮ǰ��ͨ���㲥 OnAssetPreImport �¼���֪ͨ�༭��������ϵͳ����������ָ������Դ�ࡢ���������ƺ����͡�
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	if (InClass != UCustomNormalDistribution::StaticClass() || FCString::Stricmp(Type, TEXT("cnd")) != 0)
		return nullptr;

	UCustomNormalDistribution* Data = CastChecked<UCustomNormalDistribution>(NewObject<UCustomNormalDistribution>(InParent, InName, Flags));

	// ���ļ���ȡֵ
	Data->Mean = FCString::Atof(*GetValueFromFile(Buffer, "[MySection]", "Mean"));
	Data->StandardDeviation = FCString::Atof(*GetValueFromFile(Buffer, "[MySection]", "StandardDeviation"));

	// ���ļ���ȡֵ����һ�ַ�ʽ�����⽫�ļ����ݵ���д��ʽ��.ini���ƣ�����Ҳ���Խ���.ini��ʽ����
	// FConfigCacheIni Config(EConfigCacheType::Temporary);
	// Config.LoadFile(CurrentFilename);
	// Config.GetFloat(TEXT("MySection"), TEXT("Mean"), Data->Mean, CurrentFilename);
	// Config.GetFloat(TEXT("MySection"), TEXT("StandardDeviation"), Data->StandardDeviation, CurrentFilename);

	// ���浼���·��
	Data->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, Data->GetPackage());
	// ֪ͨ�༭��������ϵͳ����ʾ�����������ɡ�
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, Data);

	return Data;
}

bool UCustomNDImportFactory::FactoryCanImport(const FString& Filename)
{
	// ���ڼ�鹤���Ƿ��ܹ�����ָ���ļ�����Դ��
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
		������Ҫʵ�ּ���߼����������µ�����ļ�·����ӵ� OutFilenames ��
		���� true ��ʾ�������µ��룬false ��ʾ���������µ���
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
		������Ҫ�������µ�����ļ�·��
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
		���ڴ�����Դ�����µ��������������� CanReimport �� SetReimportPaths ������ִ�о�������µ����߼���
		Obj��Ҫ���µ���Ķ���ͨ����֮ǰ�������Դ��
		retuen: EReimportResult::Type�����µ�������Ľ�����ͣ������ɹ���ʧ�ܵȡ�
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
