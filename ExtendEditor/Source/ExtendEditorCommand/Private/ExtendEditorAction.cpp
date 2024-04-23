// ExtendEditorAction.cpp

#pragma once

#include "ExtendEditorAction.h"

#include "ExtendEditorStyle.h"

#define LOCTEXT_NAMESPACE "FExtendEditorCommands"

TSharedRef<FUICommandList> FExtendEditorCommands::CommandList = MakeShareable(new FUICommandList);

FExtendEditorCommands::FExtendEditorCommands()
	: TCommands<FExtendEditorCommands>(
		"ExtendEditorCommands",                 // 上下文名称
		LOCTEXT("Extend Editor Commands", "Extend Editor Commands"),
		NAME_None,
		FExtendEditorStyle::GetStyleSetName())  // 样式集名称
{
}

void FExtendEditorCommands::RegisterCommands()
{
	// 设置和绑定 FUICommandInfo
	{
		// 注册 FUICommandInfo
		UI_COMMAND(PrintLog, "Print Log Label", "Print Log ToolTip",
			EUserInterfaceActionType::Button, FInputChord())

			// 等同于上文：
			// FUICommandInfo::MakeCommandInfo(this->AsShared(),UICommandInfo_MenuEntry2,...);

			// 绑定 FUICommandInfo 及其 Action 到 ActionList
			CommandList->MapAction(
				PrintLog,
				FExecuteAction::CreateStatic(&FExtendEditorActionCallbacks::PrintLog)
			);
	}

	// 继续设置和绑定 FUICommandInfo...
	{
		// ...
	}
}
#undef LOCTEXT_NAMESPACE



void FExtendEditorActionCallbacks::PrintLog()
{
	UE_LOG(LogTemp, Warning, TEXT("StyleButtonAction is called."));
}