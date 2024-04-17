// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

/**
 * 为自定义数据类型构建编辑器 
 */
class CUSTOMDATATYPEEDITOR_API FCustomNDEditorToolkit :public FAssetEditorToolkit
{
public:
	FCustomNDEditorToolkit();
	~FCustomNDEditorToolkit();


	// 这两个函数用于注册和注销编辑器中的标签生成器（Tab Spawner）。
	// 标签生成器负责生成和管理编辑器中的标签页（Tabs），它们定义了编辑器的布局结构和可见性。
	// 通过这两个函数，你可以在编辑器初始化时注册你自己的标签生成器，并在编辑器关闭时注销它们，确保编辑器的标签页可以正确地显示和管理。
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	// 返回编辑器的标识名称。这个名称通常是一个唯一的标识符，用于在 Unreal Engine 内部识别和查找编辑器。
	virtual FName GetToolkitFName() const override { return "CustomNormalDistributionEditor"; }
	// 返回编辑器的基本名称。通常用于显示在编辑器标题栏或其他 UI 元素中，向用户展示编辑器的名称。
	virtual FText GetBaseToolkitName() const override { return INVTEXT("Custom Normal Distribution Editor"); }
	// 返回编辑器在世界中心（World-Centric）的标签前缀。世界中心是 UE 中的一个概念，表示编辑器在该模式下的工作方式。这个函数返回的前缀将影响编辑器在世界中心模式下的标签显示。
	virtual FString GetWorldCentricTabPrefix() const override { return "Custom Normal Distribution Editor"; }
	// 返回编辑器在世界中心模式下的标签颜色缩放比例。这个函数返回的颜色比例将影响编辑器标签的颜色
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return {}; }


	// 外部调用的入口，它可以是任意名字，可以具有任意参数。
	// 负责规划窗口布局并调用 FAssetEditorToolkit::InitAssetEditor()。
	void InitEditor(const TArray<UObject*>& InObjects);

	float GetMean() const;
	float GetStandarDeviation() const;
	void SetMean(float InMean);
	void SetStandarDeviation(float InStandardDeviation);


private:
	class UCustomNormalDistribution* NormalDistribution = nullptr;
};
