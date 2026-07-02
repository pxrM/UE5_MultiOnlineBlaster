// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Shared log category for the whole framework. Declared in Core so every module can use it.
UIFRAMEWORKCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogUIFramework, Log, All);

class FUIFrameworkCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
