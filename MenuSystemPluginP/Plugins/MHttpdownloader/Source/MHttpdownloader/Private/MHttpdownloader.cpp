// Copyright Epic Games, Inc. All Rights Reserved.

#include "MHttpdownloader.h"

#define LOCTEXT_NAMESPACE "FMHttpdownloaderModule"

void FMHttpdownloaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMHttpdownloaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMHttpdownloaderModule, MHttpdownloader)

// ���ڶ���һ����־�������ʵ������Դ�ļ��н��ж��塣
DEFINE_LOG_CATEGORY(LogFileDownloader);