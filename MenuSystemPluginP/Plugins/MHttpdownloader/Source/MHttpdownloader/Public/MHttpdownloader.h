// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMHttpdownloaderModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// ����һ���ⲿ����־��������ͷ�ļ��н����������Ա������������п���ʹ��ͬһ����־������
DECLARE_LOG_CATEGORY_EXTERN(LogFileDownloader, Log, All);