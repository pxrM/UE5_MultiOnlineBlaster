// Source/CustomDataTypeEditor/Public/CustomDataTypeEditor.h

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomDataTypeEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};