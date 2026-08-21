// Copyright TikiStar. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "UIViewModelTestTypes.h"
#include "UnLuaBase.h"
#include "UnLuaModule.h"
#include "lua.hpp"

#include "UIFrameworkLuaTestHelpers.h"

/**
 * Exercises Content/Script/UIFramework/ViewModel.lua against a real UnLua environment
 * and real reflection data. The proxy's whole reason to exist is that a plain
 * assignment from Lua does not broadcast a field change, so this has to run through
 * actual UnLua rather than assert on C++ behavior alone.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkViewModelLuaProxyTest,
	"TikiStar.UIFramework.ViewModel.LuaProxy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkViewModelLuaProxyTest::RunTest(const FString& Parameters)
{
	const FString ModulePath = FPaths::ProjectContentDir() / TEXT("Script/UIFramework/ViewModel.lua");
	if (!FPaths::FileExists(ModulePath))
	{
		AddInfo(FString::Printf(TEXT("ViewModel.lua not installed at '%s'; skipping."), *ModulePath));
		return true;
	}

	const UIFrameworkLuaTest::FScopedUnLuaActivation Activation;
	UnLua::FLuaEnv* Env = IUnLuaModule::Get().GetEnv(nullptr);
	if (!Env)
	{
		AddInfo(TEXT("No Lua environment available in this context; skipping."));
		return true;
	}

	lua_State* L = Env->GetMainState();
	UUIViewModelTestSubject* VM = NewObject<UUIViewModelTestSubject>();

	// Count broadcasts so the test observes "the binding was told", not just the value.
	VM->StartCountingBroadcasts();

	UnLua::PushUObject(L, VM);
	lua_setglobal(L, "UIFrameworkTestVM");

	// The chunk reports failures through a global rather than erroring, so a mismatch
	// arrives as a readable message instead of a Lua traceback.
	const FString Chunk = TEXT(R"LUA(
		UIFrameworkTestError = nil
		local function fail(fmt, ...)
			UIFrameworkTestError = string.format(fmt, ...)
			error(UIFrameworkTestError, 0)
		end

		local ok, ViewModel = pcall(require, "UIFramework.ViewModel")
		if not ok then fail("require failed: %s", tostring(ViewModel)) end

		local vm = ViewModel.Wrap(UIFrameworkTestVM)

		-- Integers must not land in a float property and vice versa.
		vm.Score = 7
		if UIFrameworkTestVM.Score ~= 7 then fail("Score not written: %s", tostring(UIFrameworkTestVM.Score)) end
		vm.Ratio = 0.5
		vm.Note = "lua"
		vm.bEnabled = true

		-- Reads go through to the UObject.
		if vm.Score ~= 7 then fail("read-through failed: %s", tostring(vm.Score)) end

		-- Methods still resolve against the UObject through the proxy.
		if not vm:HasNotifyField("Score") then fail("HasNotifyField through proxy failed") end

		-- Rewriting the same value must not broadcast.
		vm.Score = 7

		-- Unwrap yields the original object.
		if ViewModel.Unwrap(vm) ~= UIFrameworkTestVM then fail("Unwrap did not return the target") end
		if ViewModel.Unwrap(UIFrameworkTestVM) ~= UIFrameworkTestVM then fail("Unwrap of a raw object failed") end

		-- SetAll reports unknown fields instead of erroring.
		local allOk, failed = ViewModel.SetAll(vm, { Score = 11 })
		if not allOk then fail("SetAll rejected a valid field: %s", tostring(failed[1])) end
		local badOk, badFailed = ViewModel.SetAll(vm, { Nonexistent = 1 })
		if badOk then fail("SetAll accepted an unknown field") end
		if badFailed[1] ~= "Nonexistent" then fail("SetAll named the wrong field: %s", tostring(badFailed[1])) end

		UIFrameworkTestResult = "ok"
	)LUA");

	// A refused SetAll field logs from the C++ side; that is the expected path.
	AddExpectedError(TEXT("is not a FieldNotify field"), EAutomationExpectedErrorFlags::Contains, 0);

	if (!UIFrameworkLuaTest::RunChunk(*this, *Env, Chunk, TEXT("UIFrameworkViewModelLuaProxyTest")))
	{
		lua_pushnil(L);
		lua_setglobal(L, "UIFrameworkTestVM");
		return false;
	}

	// --- verify from the C++ side that the proxy routed through the setters ------
	TestEqual(TEXT("An integer assignment reached the int property"), VM->Score, 11);
	TestEqual(TEXT("Score broadcast once per distinct value"), VM->GetBroadcastCount(TEXT("Score")), 2);
	TestEqual(TEXT("A fractional assignment reached the float property"), VM->Ratio, 0.5f);
	TestEqual(TEXT("A string assignment reached the string property"), VM->Note, FString(TEXT("lua")));
	TestTrue(TEXT("A boolean assignment reached the bool property"), VM->bEnabled);
	TestEqual(TEXT("An integer assignment did not leak into the double property"), VM->Precise, 0.0);

	lua_pushnil(L);
	lua_setglobal(L, "UIFrameworkTestVM");
	return true;
}

#endif
