// ExtendMenuBase.cpp

#pragma once

#include "ExtendMenuBase.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "ToolMenus.h"
#include "ExtendEditorStyle.h"
#include "ExtendEditorAction.h"
#include "ExtendEditorCommand.h"

IMPLEMENT_GAME_MODULE(FExtendMenuBase, ExtendMenuBase)

void FExtendMenuBase::StartupModule()
{
	IModuleInterface::StartupModule();

	ExtendMenuByFExtend();
	ExtendContentBrowserByFExtend();
	ExtendViewportByFExtend();

	//ExtendByUToolMenus();
	// 由FLevelEditorModule管理的拓展入口
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnLevelEditorCreated().AddRaw(this, &FExtendMenuBase::OnLevelEditorCreatedEvent);

	StyleSample();

	// 确保FExtendEditorCommandModule已加载
	FModuleManager::LoadModuleChecked<FExtendEditorCommandModule>("ExtendEditorCommand");
	CommandSample();
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

void FExtendMenuBase::ExtendViewportByFExtend()
{
	/*
	* 先获取拓展相关的 Extenders，然后 Add 自己想要添加的内容。我们可以获取到几种不同的 Extenders:
	*	GetAllLevelViewportContextMenuExtenders：Level 场景中选中 Actor 的右键菜单。
	*	GetAllLevelViewportOptionsMenuExtenders：视口选项菜单，点击视口右上角小齿轮时弹出的菜单。
	*	GetAllLevelViewportShowMenuExtenders：视口视图菜单。点击视口右上角小眼睛时弹出的菜单。
	*	GetAllLevelViewportDragDropContextMenuExtenders：按住右键并拖拽 Object 到视口中松开时弹出的菜单。
	*/

	/*向视口中 Actor 菜单添加菜单项的示例*/
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

// 将名称空间标识符 LOCTEXT_NAMESPACE 定义为 "ExtendByToolMenus"。
// 名称空间标识符通常用于帮助管理本地化文本（Localized Text），它们允许在不同的上下文中使用相同的文本字符串，以便进行国际化和本地化。
#define LOCTEXT_NAMESPACE "ExtendByToolMenus"
void FExtendMenuBase::ExtendByUToolMenus()
{
	/**
	 * Name”是使用 UToolMenus 拓展的关键。
	 * 编辑器中的几乎每个菜单都有自己的 Name。打开 Window > Enable Menu Editing 开关，可以在大多数菜单中发现 Open Menu Editor 按钮，
	 * 点击该按钮打开 Menu Editor，可以在其中找到该菜单的 Name。
	 * Name 有固定的规律：
	 *	主菜单栏：http://LevelEditor.MainMenu.XXX
	 *	主菜单栏：http://LevelEditor.MainMenu.XXX
	 *	ToolBar：http://LevelEditor.LevelEditorToolBar.XXX
	 *	ContentBrowser菜单：http://ContentBrowser.XXX
	 *	蓝图编辑器：http://AssetEditor.BlueprintEditor.XXX
	 *	材质编辑器：http://AssetEditor.MaterialEditor.XXX
	 *	如此等等...
	 * 当我们要拓展一个菜单时，需要持有该菜单的 UToolMenu，该 UToolMenu 可以通过菜单的 Name 找到。
	 * 拓展已有菜单基本的流程是：
	 *	通过 Name 持有菜单的 UToolMenu。
	 *	通过该 UToolMenu 找到或添加 FToolMenuSection。
	 *	通过 FToolMenuSection 添加菜单项。
	 *
	 */

	 // 持有LevelEditor.MainMenu
	 //UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	 // 由于调用时机改变了，我们可以使用更为严格的FindMenu()
	UToolMenu* Menu = UToolMenus::Get()->FindMenu("LevelEditor.MainMenu");

	// 定位LevelEditor.MainMenu中的Section
	// LevelEditor.MainMenu没有Section因此传入NAME_None
	FToolMenuSection& Section = Menu->FindOrAddSection(NAME_None);

	// 开始制作子菜单Entry
	FToolMenuEntry& MakeEntry = Section.AddSubMenu(
		"NewMenuByUToolMenus",// 新子菜单 Name
		LOCTEXT("NewMenu2", "New Menu 2"),      // 标签
		LOCTEXT("NewMenu2_ToolTip", "This is a extended menu by UToolMenus"), // ToolTip
		FNewToolMenuChoice()
	);
	// 设置Entry出现的位置
	MakeEntry.InsertPosition = FToolMenuInsert("Help", EToolMenuInsertType::After);

	// LevelEditor.MainMenu.NewMenuByUToolMenus注册为新菜单
	static const FName BaseMenuName = "LevelEditor.MainMenu.NewMenuByUToolMenus";
	Menu = UToolMenus::Get()->RegisterMenu(BaseMenuName);

	// 在新菜单下添加section和entry
	FToolMenuSection& NewSection = Menu->AddSection("New Section", FText::FromString("New Section"));
	NewSection.AddMenuEntry(
		"New Menu 2 Btn",
		FText::FromString("Lable: New Menu 2 Btn"),
		FText::FromString("This is a new menu 2 button by UToolMenus"),
		FSlateIcon(),
		FToolUIActionChoice(FExecuteAction::CreateRaw(this, &FExtendMenuBase::NewMenu2ButtonAction))
	);
}
#undef LOCTEXT_NAMESPACE

void FExtendMenuBase::NewMenu2ButtonAction()
{
	UE_LOG(LogTemp, Warning, TEXT("NewMenu2ButtonAction is called."));
}

void FExtendMenuBase::OnLevelEditorCreatedEvent(TSharedPtr<class ILevelEditor> Editor)
{
	ExtendByUToolMenus();
}

void FExtendMenuBase::StyleSample()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = Menu->FindOrAddSection(NAME_None);
	Section.AddEntry(
		FToolMenuEntry::InitToolBarButton(
			"StyBtn",
			FToolUIActionChoice(FExecuteAction::CreateRaw(this, &FExtendMenuBase::StyleButtonAction)),
			FText::FromString("Lable: Style Buttom"),
			FText::FromString("This is a style button by Style Sample"),
			FSlateIcon(/*使用自己注册的FSlateStyleSet设置Icon样式*/FExtendEditorStyle::GetStyleSetName(), "AliceTool")
		)
	);
}

void FExtendMenuBase::StyleButtonAction()
{
	UE_LOG(LogTemp, Warning, TEXT("StyleButtonAction is called."));
}

void FExtendMenuBase::CommandSample()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = Menu->FindOrAddSection(NAME_None);

	// 使用 Command 方式生成 ToolBarEntry
	FToolMenuEntry ToolMenuEntry = FToolMenuEntry::InitToolBarButton(FExtendEditorCommands::Get().PrintLog);
	// 为 ToolBarEntry 配置CommandList
	ToolMenuEntry.SetCommandList(FExtendEditorCommands::Get().CommandList);
	// 将配置好的 ToolBarEntry 添加到指定位置
	Section.AddEntry(ToolMenuEntry);
}
