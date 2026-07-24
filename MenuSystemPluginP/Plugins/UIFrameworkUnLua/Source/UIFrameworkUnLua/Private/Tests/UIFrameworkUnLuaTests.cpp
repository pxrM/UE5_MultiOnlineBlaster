// Copyright TikiStar. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Scripting/UIScriptBridgeSubsystem.h"
#include "UIUnLuaScreenBase.h"
#include "UnLuaInterface.h"
#include "lua.hpp"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkUnLuaAdapterTest,
	"TikiStar.UIFramework.UnLua.Adapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkUnLuaAdapterTest::RunTest(const FString& Parameters)
{
	const UClass* AdapterClass = UUIUnLuaScreenBase::StaticClass();
	TestTrue(TEXT("Adapter derives from the framework screen base"), AdapterClass->IsChildOf(UUIActivatableScreenBase::StaticClass()));
	TestTrue(TEXT("Adapter implements UnLua's static binding interface"), AdapterClass->ImplementsInterface(UUnLuaInterface::StaticClass()));
	TestNotNull(TEXT("Script bridge remains available to UnLua through reflection"), UUIScriptBridgeSubsystem::StaticClass());

	const UUIUnLuaScreenBase* DefaultScreen = GetDefault<UUIUnLuaScreenBase>();
	TestEqual(
		TEXT("Base adapter does not bind a module until a screen configures one"),
		IUnLuaInterface::Execute_GetModuleName(DefaultScreen),
		FString());

	const FString RegistryModulePath = FPaths::ProjectContentDir() / TEXT("Script/UIFramework/Registry.lua");
	TestTrue(TEXT("Lua table registry module is installed"), FPaths::FileExists(RegistryModulePath));
	lua_State* LuaState = luaL_newstate();
	const int32 LoadResult = luaL_loadfile(LuaState, TCHAR_TO_UTF8(*RegistryModulePath));
	if (LoadResult != LUA_OK)
	{
		AddError(FString::Printf(TEXT("Registry.lua syntax error: %s"), UTF8_TO_TCHAR(lua_tostring(LuaState, -1))));
	}
	lua_close(LuaState);
	TestEqual(TEXT("Lua table registry module compiles"), LoadResult, static_cast<int32>(LUA_OK));
	return true;
}

#endif
