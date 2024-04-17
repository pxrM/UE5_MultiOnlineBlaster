// Source/CustomDataTypeEditor/Private/CustomDataTypeEditor.cpp

#pragma once

#include "CustomDataTypeEditor.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"

IMPLEMENT_MODULE(FCustomDataTypeEditorModule, CustomDataTypeEditor)

void FCustomDataTypeEditorModule::StartupModule()
{
	// ע���µ� Category
	// ����ע����һ����Ϊ "Example" �ĸ߼���Դ��𣬲�ָ����һ����Ӧ��˵���ı���
	EAssetTypeCategories::Type Category = FAssetToolsModule::GetModule().Get().RegisterAdvancedAssetCategory(FName(TEXT("Example")), FText::FromString("Example"));

	// ע�� AssetTypeActions
	// ������һ�� FCustomNormalDistributionActions ���ʵ����ͬʱ��֮ǰע�����Դ��� Category ��Ϊ�������ݸ��˹��캯����
	CustomNormalDistributionActionsPtr = MakeShared<FCustomNormalDistributionActions>(Category);
	// ������Զ������Դ���Ͳ���ע�ᵽ�� Unreal Engine ����Դ����ģ���У��Ա��ڱ༭������ȷ��ʾ�͹�������͵���Դ��
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(CustomNormalDistributionActionsPtr.ToSharedRef());
}

void FCustomDataTypeEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(CustomNormalDistributionActionsPtr.ToSharedRef());
	}
}