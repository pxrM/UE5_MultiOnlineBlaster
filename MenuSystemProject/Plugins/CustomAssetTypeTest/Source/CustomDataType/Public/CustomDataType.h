// Source/CustomDataType/Public/CustomDataType.h

// �Զ�������

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomDataTypeModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};