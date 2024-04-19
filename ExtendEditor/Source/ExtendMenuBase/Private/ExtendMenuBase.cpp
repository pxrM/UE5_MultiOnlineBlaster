// ExtendMenuBase.cpp

#pragma once

#include "ExtendMenuBase.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "ToolMenus.h"

IMPLEMENT_GAME_MODULE(FExtendMenuBase, ExtendMenuBase)

void FExtendMenuBase::StartupModule()
{
	IModuleInterface::StartupModule();

	ExtendMenuByFExtend();
	ExtendContentBrowserByFExtend();
	ExtendViewportByFExtend();
	ExtendByUToolMenus();
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

void FExtendMenuBase::ExtendViewportByFExtend()
{
	/*
	* �Ȼ�ȡ��չ��ص� Extenders��Ȼ�� Add �Լ���Ҫ��ӵ����ݡ����ǿ��Ի�ȡ�����ֲ�ͬ�� Extenders:
	*	GetAllLevelViewportContextMenuExtenders��Level ������ѡ�� Actor ���Ҽ��˵���
	*	GetAllLevelViewportOptionsMenuExtenders���ӿ�ѡ��˵�������ӿ����Ͻ�С����ʱ�����Ĳ˵���
	*	GetAllLevelViewportShowMenuExtenders���ӿ���ͼ�˵�������ӿ����Ͻ�С�۾�ʱ�����Ĳ˵���
	*	GetAllLevelViewportDragDropContextMenuExtenders����ס�Ҽ�����ק Object ���ӿ����ɿ�ʱ�����Ĳ˵���
	*/

	/*���ӿ��� Actor �˵���Ӳ˵����ʾ��*/
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelViewportContextMenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	LevelViewportContextMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(this, &FExtendMenuBase::ExtendViewportSelectedActorMenu));
}

TSharedRef<FExtender> FExtendMenuBase::ExtendViewportSelectedActorMenu(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedActors.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			"ActorOptions",
			EExtensionHook::Before,
			nullptr,
			FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeViewportActorMenuEntry)
		);
	}
	return MenuExtender;
}

void FExtendMenuBase::MakeViewportActorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Viewport Actor Menu Button"),
		FText::FromString("This is a viewport actor menu button"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::ViewportActorMenuEntryAction)));
}

void FExtendMenuBase::ViewportActorMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ViewportActorMenuEntryAction is called."));
}

// �����ƿռ��ʶ�� LOCTEXT_NAMESPACE ����Ϊ "ExtendByToolMenus"��
// ���ƿռ��ʶ��ͨ�����ڰ��������ػ��ı���Localized Text�������������ڲ�ͬ����������ʹ����ͬ���ı��ַ������Ա���й��ʻ��ͱ��ػ���
#define LOCTEXT_NAMESPACE "ExtendByToolMenus"
void FExtendMenuBase::ExtendByUToolMenus()
{
	/**
	 * Name����ʹ�� UToolMenus ��չ�Ĺؼ���
	 * �༭���еļ���ÿ���˵������Լ��� Name���� Window > Enable Menu Editing ���أ������ڴ�����˵��з��� Open Menu Editor ��ť��
	 * ����ð�ť�� Menu Editor�������������ҵ��ò˵��� Name��
	 * Name �й̶��Ĺ��ɣ�
	 *	���˵�����http://LevelEditor.MainMenu.XXX
	 *	���˵�����http://LevelEditor.MainMenu.XXX
	 *	ToolBar��http://LevelEditor.LevelEditorToolBar.XXX
	 *	ContentBrowser�˵���http://ContentBrowser.XXX
	 *	��ͼ�༭����http://AssetEditor.BlueprintEditor.XXX
	 *	���ʱ༭����http://AssetEditor.MaterialEditor.XXX
	 *	��˵ȵ�...
	 * ������Ҫ��չһ���˵�ʱ����Ҫ���иò˵��� UToolMenu���� UToolMenu ����ͨ���˵��� Name �ҵ���
	 * ��չ���в˵������������ǣ�
	 *	ͨ�� Name ���в˵��� UToolMenu��
	 *	ͨ���� UToolMenu �ҵ������ FToolMenuSection��
	 *	ͨ�� FToolMenuSection ��Ӳ˵��
	 * 
	 */

	// ����LevelEditor.MainMenu
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	// ��λLevelEditor.MainMenu�е�Section
	// LevelEditor.MainMenuû��Section��˴���NAME_None
	FToolMenuSection& Section = Menu->FindOrAddSection(NAME_None);
	// ��ʼ�����Ӳ˵�Entry
	FToolMenuEntry& MakeEntry = Section.AddSubMenu(
		"NewMenuByUToolMenus",// ���Ӳ˵� Name
		LOCTEXT("NewMenu2", "New Menu 2"),      // ��ǩ
		LOCTEXT("NewMenu2_ToolTip", "This is a extended menu by UToolMenus"), // ToolTip
		FNewToolMenuChoice()
	);
	// ����Entry���ֵ�λ��
	MakeEntry.InsertPosition = FToolMenuInsert("Help", EToolMenuInsertType::After);
}
#undef LOCTEXT_NAMESPACE

void FExtendMenuBase::NewMenu2ButtonAction()
{
}
