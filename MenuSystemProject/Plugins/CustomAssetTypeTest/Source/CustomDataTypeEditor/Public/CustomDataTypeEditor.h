// Source/CustomDataTypeEditor/Public/CustomDataTypeEditor.h

// �����Զ�������

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
	// ��¼ע���AssetTypeActions�Թ�ģ��ͣ��ʱж��
	TSharedPtr<FCustomNormalDistributionActions> CustomNormalDistributionActionsPtr;
};