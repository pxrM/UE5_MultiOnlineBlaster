// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * FAssetTypeActions_Base 是所有 AssetTypeActions 的基类，提供有关特定资产类型的操作和其他信息。
 * 它是可选的，但一般默认需要提供，否则无法完成诸如双击打开资产编辑器等操作。
 * 
 * 当编辑器在 ContentBrowser 中处理资产操作（创建资产、双击打开编辑资源等）时，会搜寻是否有对应的 AssetTypeActionsClass 被注册。
 * 
 * AssetTypeActions 中有各种与资产操作以及资产信息显示有关的虚函数，我们可以通过覆写它们来自定义资产操作的行为和资产显示方式。
 */
class CUSTOMDATATYPEEDITOR_API FCustomNormalDistributionActions :public FAssetTypeActions_Base
{
public:
	FCustomNormalDistributionActions(EAssetTypeCategories::Type InAssetCategory);
	~FCustomNormalDistributionActions();

	// 返回此操作支持的资源类的类
	virtual UClass* GetSupportedClass() const override;
	// 返回此操作的名称，通常是用于在用户界面中显示的文本。
	virtual FText GetName() const override;
	// 返回此操作的类型颜色，通常用于在用户界面中标识不同类型的资源。
	virtual FColor GetTypeColor() const override;
	// 返回此操作所属的资源类型类别。
	virtual uint32 GetCategories() override;

	// 根据给定的资源对象列表构建右键操作菜单。
	virtual void GetActions(const TArray<UObject*>& InObjects, class FMenuBuilder& MenuBuilder) override;
	// 检查给定的资源对象列表中是否有可用的操作。
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;

	
	/*
	* 为自定义数据类型构建编辑器
	*	双击资产打开编辑器这个操作的入口在 UAssetEditorSubsystem::OpenEditorForAsset()
	*	接着会进入 FAssetTypeActions_Base::OpenAssetEditor -> FSimpleAssetEditor::CreateEditor -> FSimpleAssetEditor::InitEditor...
	* 在自己的 AssetTypeActions 中重写 OpenAssetEditor()，然后用它调用自定义的 FAssetEditorToolkit 类，来打开自己的编辑器。
	* 
	* InObjects：一个 TArray 类型的对象数组，包含需要编辑的资产对象。
	*/
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /* = TSharedPtr<IToolkitHost>() */) override;

	// 重新导入
	void ExecuteReimport(const TArray<UObject*>& InObjects);

private:
	EAssetTypeCategories::Type AssetCategory;

};
