// ExtendEditorAction.h

#pragma once

#include "Framework/Commands/UICommandList.h"

class EXTENDEDITORCOMMAND_API FExtendEditorCommands : public TCommands<FExtendEditorCommands>
{
public:
	
	FExtendEditorCommands();

	virtual void RegisterCommands() override;

public:

	// FUICommandList成对地映射 UICommandInfo与其 Action
	static TSharedRef< FUICommandList > CommandList;

public:
	
	// UICommandInfo List
	// FUICommandInfo储存 Command样式信息
	TSharedPtr< FUICommandInfo > PrintLog;
};


/**
 * Action合集类
 */
class FExtendEditorActionCallbacks
{
public:
	// Action List
	static void PrintLog();
	
};