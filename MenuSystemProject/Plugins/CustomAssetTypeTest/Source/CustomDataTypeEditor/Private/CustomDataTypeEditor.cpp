// Source/CustomDataTypeEditor/Private/CustomDataTypeEditor.cpp

#pragma once

#include "CustomDataTypeEditor.h"

IMPLEMENT_MODULE(FCustomDataTypeEditorModule, CustomDataTypeEditor)

void FCustomDataTypeEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
}

void FCustomDataTypeEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}