// Copyright TikiStar. All Rights Reserved.

#include "UIFrameworkCoreModule.h"

DEFINE_LOG_CATEGORY(LogUIFramework);

void FUIFrameworkCoreModule::StartupModule()
{
	UE_LOG(LogUIFramework, Log, TEXT("UIFrameworkCore started."));
}

void FUIFrameworkCoreModule::ShutdownModule()
{
	UE_LOG(LogUIFramework, Log, TEXT("UIFrameworkCore shut down."));
}

IMPLEMENT_MODULE(FUIFrameworkCoreModule, UIFrameworkCore);
