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

// 声明一个外部的日志类别对象，在头文件中进行声明，以便在整个程序中可以使用同一个日志类别对象。
DECLARE_LOG_CATEGORY_EXTERN(LogFileDownloader, Log, All);