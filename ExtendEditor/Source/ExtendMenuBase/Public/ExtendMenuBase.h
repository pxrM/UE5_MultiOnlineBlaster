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

};