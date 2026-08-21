// Copyright TikiStar. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Config/UIWidgetRegistry.h"
#include "Scripting/UIScriptBridgeSubsystem.h"
#include "Scripting/UIScriptClassBinder.h"
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

	// The module registers the binder on startup, which is what makes a registry
	// ScriptModule field do anything at all.
	TestNotNull(TEXT("Adapter installed a script class binder"), UIFrameworkScript::GetClassBinder());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkScriptModuleValidationTest,
	"TikiStar.UIFramework.UnLua.ScriptModuleValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkScriptModuleValidationTest::RunTest(const FString& Parameters)
{
	const FGameplayTag KeyA = FGameplayTag::RequestGameplayTag(TEXT("UI.Menu"), false);
	const FGameplayTag KeyB = FGameplayTag::RequestGameplayTag(TEXT("UI.Test"), false);
	if (!KeyA.IsValid() || !KeyB.IsValid())
	{
		AddInfo(TEXT("UI.Menu / UI.Test tags are not registered in this project; skipping."));
		return true;
	}

	const FSoftObjectPath SharedClass(TEXT("/Game/Blueprint/UI/WBP_TestScreen.WBP_TestScreen_C"));

	FUIWidgetEntry Suffixed;
	Suffixed.WidgetClass = TSoftClassPtr<UUserWidget>(SharedClass);
	Suffixed.ScriptModule = TEXT("UI.TestScreen.lua");
	TArray<FString> Errors;
	ValidateUIWidgetEntry(KeyA, Suffixed, Errors);
	TestFalse(TEXT("A .lua suffix in ScriptModule is rejected"), Errors.IsEmpty());

	FUIWidgetEntry Slashed = Suffixed;
	Slashed.ScriptModule = TEXT("UI/TestScreen");
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, Slashed, Errors);
	TestFalse(TEXT("Path separators in ScriptModule are rejected"), Errors.IsEmpty());

	FUIWidgetEntry Valid = Suffixed;
	Valid.ScriptModule = TEXT("UI.TestScreen");
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, Valid, Errors);
	TestTrue(TEXT("A dotted, suffix-free ScriptModule is accepted"), Errors.IsEmpty());

	// Binding targets the constructed widget, so a module with no class is dead config.
	FUIWidgetEntry NoClass;
	NoClass.ScriptModule = TEXT("UI.TestScreen");
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, NoClass, Errors);
	TestFalse(TEXT("A ScriptModule without a WidgetClass is rejected"), Errors.IsEmpty());

	// KeepUntilIdle with a non-positive TTL is caching that never caches.
	FUIWidgetEntry ZeroTtl = Valid;
	ZeroTtl.CachePolicy = EUICachePolicy::KeepUntilIdle;
	ZeroTtl.IdleTimeoutSeconds = 0.f;
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, ZeroTtl, Errors);
	TestFalse(TEXT("KeepUntilIdle with a zero TTL is rejected"), Errors.IsEmpty());

	ZeroTtl.IdleTimeoutSeconds = 10.f;
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, ZeroTtl, Errors);
	TestTrue(TEXT("KeepUntilIdle with a positive TTL is accepted"), Errors.IsEmpty());

	// Policies that ignore the field must not trip the TTL rule.
	FUIWidgetEntry PersistentZeroTtl = Valid;
	PersistentZeroTtl.CachePolicy = EUICachePolicy::KeepPersistent;
	PersistentZeroTtl.IdleTimeoutSeconds = 0.f;
	Errors.Reset();
	ValidateUIWidgetEntry(KeyA, PersistentZeroTtl, Errors);
	TestTrue(TEXT("A zero TTL is ignored by policies that do not use it"), Errors.IsEmpty());

	// Same class, different modules: UnLua binds per UClass, so this cannot work.
	FUIWidgetEntry Conflicting = Valid;
	Conflicting.ScriptModule = TEXT("UI.OtherScreen");
	TMap<FGameplayTag, FUIWidgetEntry> Conflict;
	Conflict.Add(KeyA, Valid);
	Conflict.Add(KeyB, Conflicting);
	Errors.Reset();
	ValidateUIScriptModuleUniqueness(Conflict, Errors);
	TestFalse(TEXT("Two modules on one widget class are rejected"), Errors.IsEmpty());

	// Same class, same module: two keys may legitimately share one screen.
	TMap<FGameplayTag, FUIWidgetEntry> Shared;
	Shared.Add(KeyA, Valid);
	Shared.Add(KeyB, Valid);
	Errors.Reset();
	ValidateUIScriptModuleUniqueness(Shared, Errors);
	TestTrue(TEXT("One module shared by two keys is allowed"), Errors.IsEmpty());

	// Registry.lua ships with the host project, not the plugin, so a project that
	// does not use the Lua table registry is not a test failure.
	const FString RegistryModulePath = FPaths::ProjectContentDir() / TEXT("Script/UIFramework/Registry.lua");
	if (!FPaths::FileExists(RegistryModulePath))
	{
		AddInfo(FString::Printf(
			TEXT("Lua table registry module not installed at '%s'; skipping compile check."),
			*RegistryModulePath));
		return true;
	}

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
