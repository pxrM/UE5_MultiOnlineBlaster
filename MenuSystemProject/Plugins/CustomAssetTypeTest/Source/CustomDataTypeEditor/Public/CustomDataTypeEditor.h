// Source/CustomDataTypeEditor/Public/CustomDataTypeEditor.h

// 操作自定义数据

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "CustomNormalDistributionActions.h"

class FCustomDataTypeEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// 记录注册的AssetTypeActions以供模块停用时卸载
	TSharedPtr<FCustomNormalDistributionActions> CustomNormalDistributionActionsPtr;
};