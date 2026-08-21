// Copyright TikiStar. All Rights Reserved.

#include "UIFrameworkUnLuaModule.h"
#include "Scripting/UIScriptClassBinder.h"

void FUIFrameworkUnLuaModule::StartupModule()
{
	UIFrameworkScript::RegisterClassBinder(&ClassBinder);
}

void FUIFrameworkUnLuaModule::ShutdownModule()
{
	UIFrameworkScript::UnregisterClassBinder(&ClassBinder);
	ClassBinder.Reset();
}

IMPLEMENT_MODULE(FUIFrameworkUnLuaModule, UIFrameworkUnLua)
