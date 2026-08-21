// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "UIUnLuaClassBinder.h"

class FUIFrameworkUnLuaModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Installed into UIFrameworkWidgets so registry entries can name a Lua module. */
	FUIUnLuaClassBinder ClassBinder;
};
