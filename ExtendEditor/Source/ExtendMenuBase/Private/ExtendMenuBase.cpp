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
	* FExtender ��չ���˵��Ļ��������ǣ�
	* ����һ�� FExtender ����
	* ����Ҫ��չ���������õ��ö����С�
	* ���ö�����ӵ��� FExtensibilityManager ����� Extenders �С�
	*/

	// ���˵���ť�µ��Ӱ�ť
	TSharedPtr<FExtender> MainMenuExtender = MakeShared<FExtender>();
	MainMenuExtender->AddMenuExtension(
		// ���ڶ�λ��ExtensionHook����չ�ҹ����༭����չ����������Զ��幦�ܻ��޸ı༭����Ϊ�Ĺҹ��㡣ͨ����չ�ҹ��������߿����ڱ༭����ע���Զ����߼����Ӷ�ʵ�ֶԱ༭���Ķ��ƺ���ǿ����
		"EpicGamesHelp",
		EExtensionHook::After,// Ӧ��ӵ��ľ���λ��(��Hook֮ǰ����֮��)
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeExistingMainMenuEntry)// ��������ť�Ĵ�����
	);

	// ���˵���ť
	MainMenuExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeNewMenuBarEntry)
	);

	// ToolBar�ϵİ�ť
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
		FText::FromString("Existing Hook Btn"),// ��ť����
		FText::FromString("This is a existing hook extend"),// ��ť��ʾ
		FSlateIcon(), // ���ǿ���Ϊ��ť���ͼ�꣬������ʱȱʡ
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::ExistingMainMenuEntryAction))// ��ִ�а�ť�Ĵ�����
	);
}

void FExtendMenuBase::ExistingMainMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ExistingMainMenuEntryAction is called."));
}

void FExtendMenuBase::MakeNewMenuBarEntry(FMenuBarBuilder& MenuBarBuilder)
{
	// ��������˵�
	MenuBarBuilder.AddPullDownMenu(
		FText::FromString("New Menu"),//��ť����
		FText::FromString("This is a new menu bar button"),//��ť����
		FNewMenuDelegate::CreateRaw(this, &FExtendMenuBase::MakeNewMainMenuEntry),
		"New Menu Hook"// �ð�ť��ExtensionHook
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
	 * ���� FContentBrowserModule ģ�顣
	 ͨ�� GetAll***Extenders() ������ȡ ContentBrowser ��Ӧ���ֵ� Extenders��
	 ע���Լ��� FExtender �� Extenders��
	 */

	 // �µ�FContentBrowserModuleģ��
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	// ͨ��GetAllAssetContextMenuExtenders��ȡExtenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& Extenders = ContentBrowserModule.GetAllAssetContextMenuExtenders();
	// ע��˴���Ӵ���Ӧ��Extenders�ж�Ӧ
	Extenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FExtendMenuBase::ExtendAssetContextMenu));

	// ͨ��AssetViewContextMenuExtenders��չѡ���ʲ������Ĳ˵�
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
