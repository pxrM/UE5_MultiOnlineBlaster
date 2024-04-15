// Source/CustomDataType/Private/CustomDataType.cpp

#pragma once

#include "CustomDataType.h"

IMPLEMENT_MODULE(FCustomDataTypeModule, CustomDataType)


void FCustomDataTypeModule::StartupModule()
{
	IModuleInterface::StartupModule();
}

void FCustomDataTypeModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}