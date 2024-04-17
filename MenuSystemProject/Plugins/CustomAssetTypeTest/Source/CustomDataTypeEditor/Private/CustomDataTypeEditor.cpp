// Source/CustomDataTypeEditor/Private/CustomDataTypeEditor.cpp

#pragma once

#include "CustomDataTypeEditor.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"

IMPLEMENT_MODULE(FCustomDataTypeEditorModule, CustomDataTypeEditor)

void FCustomDataTypeEditorModule::StartupModule()
{
	// 注册新的 Category
	// 函数注册了一个名为 "Example" 的高级资源类别，并指定了一个相应的说明文本。
	EAssetTypeCategories::Type Category = FAssetToolsModule::GetModule().Get().RegisterAdvancedAssetCategory(FName(TEXT("Example")), FText::FromString("Example"));

	// 注册 AssetTypeActions
	// 创建了一个 FCustomNormalDistributionActions 类的实例，同时将之前注册的资源类别 Category 作为参数传递给了构造函数。
	CustomNormalDistributionActionsPtr = MakeShared<FCustomNormalDistributionActions>(Category);
	// 将这个自定义的资源类型操作注册到了 Unreal Engine 的资源工具模块中，以便在编辑器中正确显示和管理该类型的资源。
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(CustomNormalDistributionActionsPtr.ToSharedRef());
}

void FCustomDataTypeEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(CustomNormalDistributionActionsPtr.ToSharedRef());
	}
}