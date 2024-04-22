// ExtendMenuBase.h

#pragma once

#include "Modules/ModuleInterface.h"

class FExtendMenuBase : public IModuleInterface
{
public:
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual ~FExtendMenuBase() {}
	
private:
	/** FExtend拓展方式入口函数 */
	void ExtendMenuByFExtend();

	/** 在已有拓展点生成一个菜单按钮 */
	void MakeExistingMainMenuEntry(FMenuBuilder& MenuBuilder);
	/** 在已有拓展点生成菜单按钮的调用事件 */
	void ExistingMainMenuEntryAction();

	/** 生成一个MenuBar菜单按钮 */
	void MakeNewMenuBarEntry(FMenuBarBuilder& MenuBarBuilder);
	/** 在新拓展点生成一个菜单按钮 */
	void MakeNewMainMenuEntry(FMenuBuilder& MenuBuilder);
	/** 在新拓展点生成菜单按钮的调用事件 */
	void NewMainMenuEntryAction();

	/** 生成一个ToolBar按钮 */
	void MakeToolBarEntry(FToolBarBuilder& ToolBarBuilder);
	/** 在ToolBar按钮点击时触发事件 */
	void ToolBarEntryAction();




	/** FExtend拓展ContentBrowser入口函数 */
	void ExtendContentBrowserByFExtend();

	/** 拓展资产面板上下文菜单 */
	TSharedRef<FExtender> ExtendAssetContextMenu(const TArray<FString>& SelectedPaths);
	/** 生成一个资产面板上下文菜单按钮 */
	void MakeAssetContextMenuEntry(FMenuBuilder& MenuBuilder);
	/** 新资产面板上下文菜单按钮触发事件 */
	void AssetContextMenuEntryAction();

	/** 拓展选定资产上下文菜单 */
	TSharedRef<FExtender> ExtendAssetViewContextMenu(const TArray<FAssetData>& SelectedAssets);
	/** 生成一个选定资产上下文菜单按钮 */
	void MakeAssetViewContextMenuEntry(FMenuBuilder& MenuBuilder);
	/** 新选定资产上下文菜单按钮触发事件 */
	void AssetViewContextMenuEntryAction();




	/** 拓展视口菜单入口函数 */
	void ExtendViewportByFExtend();
	/** 拓展视口中 Actor 菜单 */
	TSharedRef<FExtender> ExtendViewportSelectedActorMenu(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	/** 生成一个视口中 Actor 菜单按钮 */
	void MakeViewportActorMenuEntry(FMenuBuilder& MenuBuilder);
	/** 菜单触发事件 */
	void ViewportActorMenuEntryAction();




	/** UToolMenus拓展方式入口函数 */
	void ExtendByUToolMenus();

	/** NewMenu2Button触发事件 */
	void NewMenu2ButtonAction();

	/** 由FLevelEditorModule管理的拓展入口 */
	void OnLevelEditorCreatedEvent(TSharedPtr<class ILevelEditor> Editor);



	/** Style示例入口 */
	void StyleSample();
	void StyleButtonAction();
};