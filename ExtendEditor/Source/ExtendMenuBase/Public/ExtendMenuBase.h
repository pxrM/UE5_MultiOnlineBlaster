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

};