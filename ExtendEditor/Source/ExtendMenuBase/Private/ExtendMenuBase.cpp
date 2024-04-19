// ExtendMenuBase.cpp

#pragma once

#include "ExtendMenuBase.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"

IMPLEMENT_GAME_MODULE(FExtendMenuBase, ExtendMenuBase)

void FExtendMenuBase::StartupModule()
{
	IModuleInterface::StartupModule();

	ExtendMenuByFExtend();
	ExtendContentBrowserByFExtend();
}

void FExtendMenuBase::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

void FExtendMenuBase::ExtendMenuByFExtend()
{
	/*
	* FExtender 拓展主菜单的基本流程是：
	* 创建一个 FExtender 对象。
	* 将需要拓展的内容设置到该对象中。
	* 将该对象添加到由 FExtensibilityManager 管理的 Extenders 中。
	*/

	// 主菜单按钮下的子按钮
	TSharedPtr<FExtender> MainMenuExtender = MakeShared<FExtender>();
	MainMenuExtender->AddMenuExtension(
		// 用于定位的ExtensionHook（扩展挂钩，编辑器扩展中用于添加自定义功能或修改编辑器行为的挂钩点。通过扩展挂钩，开发者可以在编辑器中注入自定义逻辑，从而实现对编辑器的定制和增强。）
		"EpicGamesHelp",
		EExtensionHook::After,// 应添加到的具体位置(该Hook之前还是之后)
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeExistingMainMenuEntry)// 绑定制作按钮的代理函数
	);

	// 主菜单按钮
	MainMenuExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeNewMenuBarEntry)
	);

	// ToolBar上的按钮
	MainMenuExtender->AddToolBarExtension(
		"ProjectSettings",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeToolBarEntry)
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> MenuExtensibilityMgr = LevelEditorModule.GetMenuExtensibilityManager();

	MenuExtensibilityMgr->AddExtender(MainMenuExtender);

}

void FExtendMenuBase::MakeExistingMainMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Existing Hook Btn"),// 按钮名称
		FText::FromString("This is a existing hook extend"),// 按钮提示
		FSlateIcon(), // 我们可以为按钮添加图标，这里暂时缺省
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::ExistingMainMenuEntryAction))// 绑定执行按钮的代理函数
	);
}

void FExtendMenuBase::ExistingMainMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ExistingMainMenuEntryAction is called."));
}

void FExtendMenuBase::MakeNewMenuBarEntry(FMenuBarBuilder& MenuBarBuilder)
{
	// 添加下拉菜单
	MenuBarBuilder.AddPullDownMenu(
		FText::FromString("New Menu"),//按钮名称
		FText::FromString("This is a new menu bar button"),//按钮介绍
		FNewMenuDelegate::CreateRaw(this, &FExtendMenuBase::MakeNewMainMenuEntry),
		"New Menu Hook"// 该按钮的ExtensionHook
	);
}

void FExtendMenuBase::MakeNewMainMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Section 1", FText::FromString("Section 1"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("New Menu Btn"),
		FText::FromString("This is a new menu btn"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::NewMainMenuEntryAction))
	);
}

void FExtendMenuBase::NewMainMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("NewMainMenuEntryAction is called."));
}

void FExtendMenuBase::MakeToolBarEntry(FToolBarBuilder& ToolBarBuilder)
{
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::ToolBarEntryAction)));
}

void FExtendMenuBase::ToolBarEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ToolBarEntryAction is called."));
}

void FExtendMenuBase::ExtendContentBrowserByFExtend()
{
	/**
	 * 加载 FContentBrowserModule 模块。
	 通过 GetAll***Extenders() 函数获取 ContentBrowser 对应部分的 Extenders。
	 注入自己的 FExtender 到 Extenders。
	 */

	 // 新的FContentBrowserModule模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	// 通过GetAllAssetContextMenuExtenders获取Extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& Extenders = ContentBrowserModule.GetAllAssetContextMenuExtenders();
	// 注意此处添加代理应与Extenders中对应
	Extenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FExtendMenuBase::ExtendAssetContextMenu));

	// 通过AssetViewContextMenuExtenders拓展选定资产上下文菜单
	TArray<FContentBrowserMenuExtender_SelectedAssets>& Extenders2 = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	Extenders2.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FExtendMenuBase::ExtendAssetViewContextMenu));
}

TSharedRef<FExtender> FExtendMenuBase::ExtendAssetContextMenu(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			"ContentBrowserGetContent",
			EExtensionHook::Before,
			nullptr,
			FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeAssetContextMenuEntry)
		);
	}
	return MenuExtender;
}

void FExtendMenuBase::MakeAssetContextMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Asset Context Menu Button"),
		FText::FromString("This is a asset context menu button"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::AssetContextMenuEntryAction))
	);
}

void FExtendMenuBase::AssetContextMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("AssetContextMenuEntryAction is called."));
}

TSharedRef<FExtender> FExtendMenuBase::ExtendAssetViewContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedAssets.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::Before,
			nullptr,
			FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeAssetViewContextMenuEntry)
		);
	}
	return MenuExtender;
}

void FExtendMenuBase::MakeAssetViewContextMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Section 1", FText::FromString("Section 1"));
	MenuBuilder.AddMenuEntry(
		FText::FromString("Asset View Context Menu Button"),
		FText::FromString("This is a asset view context menu button"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::AssetViewContextMenuEntryAction))
	);

	MenuBuilder.EndSection();
}

void FExtendMenuBase::AssetViewContextMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("AssetViewContextMenuEntryAction is called."));
}
