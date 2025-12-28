#include "UMGReflectorCommands.h"

#define LOCTEXT_NAMESPACE "FUMGReflectorModule"

void FUMGReflectorCommands::RegisterCommands()
{
	/**
	* 命令注册函数
	* 命令名称：OpenPluginWindow
	* 详细描述："Open UMG Reflector"
	* 类型：按钮（EUserInterfaceActionType::Button）
	* 快捷键：无
	*/
	UI_COMMAND(OpenPluginWindow, "UMGReflector", "Open UMG Reflector", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
