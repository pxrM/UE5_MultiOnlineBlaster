// ExtendEditorCommand.cpp 

#pragma once

#include "ExtendEditorCommand.h"
#include "ExtendEditorAction.h"

IMPLEMENT_MODULE(FExtendEditorCommandModule, ExtendEditorCommand)

void FExtendEditorCommandModule::StartupModule()
{
	FExtendEditorCommands::Register();
}

void FExtendEditorCommandModule::ShutdownModule()
{
	FExtendEditorCommands::Unregister();
}