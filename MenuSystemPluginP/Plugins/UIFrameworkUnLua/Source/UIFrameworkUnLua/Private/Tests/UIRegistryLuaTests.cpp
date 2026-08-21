// Copyright TikiStar. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "UnLuaModule.h"
#include "lua.hpp"

#include "UIFrameworkLuaTestHelpers.h"

/**
 * Runs Content/Script/UIFramework/Registry.lua inside a real UnLua environment.
 *
 * Registry.Validate is pure Lua and needs no bridge, so every validation branch is
 * reachable from an automation test. That matters because these rules exist to stop
 * a malformed table from half-writing the runtime overlay — a path that is awkward
 * to reach once the bridge is involved.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkLuaRegistryValidateTest,
	"TikiStar.UIFramework.UnLua.RegistryValidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkLuaRegistryValidateTest::RunTest(const FString& Parameters)
{
	const FString ModulePath = FPaths::ProjectContentDir() / TEXT("Script/UIFramework/Registry.lua");
	if (!FPaths::FileExists(ModulePath))
	{
		AddInfo(FString::Printf(TEXT("Registry.lua not installed at '%s'; skipping."), *ModulePath));
		return true;
	}

	const UIFrameworkLuaTest::FScopedUnLuaActivation Activation;
	UnLua::FLuaEnv* Env = IUnLuaModule::Get().GetEnv(nullptr);
	if (!Env)
	{
		AddInfo(TEXT("No Lua environment available in this context; skipping."));
		return true;
	}

	const FString Chunk = TEXT(R"LUA(
		UIFrameworkTestError = nil
		local function fail(fmt, ...)
			UIFrameworkTestError = string.format(fmt, ...)
			error(UIFrameworkTestError, 0)
		end

		local ok, Registry = pcall(require, "UIFramework.Registry")
		if not ok then fail("require failed: %s", tostring(Registry)) end

		local CLASS = "/Game/Blueprint/UI/WBP_TestScreen.WBP_TestScreen_C"

		local function accepts(label, definitions)
			local good, errors = Registry.Validate(definitions)
			if not good then
				fail("%s should validate but got: %s", label, tostring(errors[1]))
			end
		end

		local function rejects(label, definitions, expectedFragment)
			local good, errors = Registry.Validate(definitions)
			if good then fail("%s should have been rejected", label) end
			if #errors == 0 then fail("%s reported no error message", label) end
			if expectedFragment and not tostring(errors[1]):find(expectedFragment, 1, true) then
				fail("%s error did not mention '%s': %s", label, expectedFragment, tostring(errors[1]))
			end
		end

		-- Minimal entry: everything but WidgetClass has a default.
		accepts("a minimal entry", { ["UI.Test"] = { WidgetClass = CLASS } })

		-- Every layer and cache policy name the enums expose must resolve.
		for name in pairs(Registry.Layers) do
			accepts("layer " .. name, { ["UI.Test"] = { WidgetClass = CLASS, Layer = name } })
		end
		for name in pairs(Registry.CachePolicies) do
			accepts("cache policy " .. name, { ["UI.Test"] = { WidgetClass = CLASS, CachePolicy = name } })
		end

		-- Structural rejections.
		rejects("a missing WidgetClass", { ["UI.Test"] = {} }, "requires WidgetClass")
		rejects("an empty WidgetClass", { ["UI.Test"] = { WidgetClass = "" } }, "requires WidgetClass")
		rejects("a non-table entry", { ["UI.Test"] = "nope" }, "must be a table")
		rejects("an empty key", { [""] = { WidgetClass = CLASS } }, "non-empty")
		rejects("an unknown layer",
			{ ["UI.Test"] = { WidgetClass = CLASS, Layer = "Nowhere" } }, "invalid Layer")
		rejects("an unknown cache policy",
			{ ["UI.Test"] = { WidgetClass = CLASS, CachePolicy = "Forever" } }, "invalid CachePolicy")

		-- ScriptModule format rules.
		accepts("a dotted ScriptModule",
			{ ["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI.TestScreen" } })
		rejects("a .lua suffix",
			{ ["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI.TestScreen.lua" } }, ".lua suffix")
		rejects("a forward slash",
			{ ["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI/TestScreen" } }, "path separators")
		rejects("a backslash",
			{ ["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI\\TestScreen" } }, "path separators")
		rejects("a non-string ScriptModule",
			{ ["UI.Test"] = { WidgetClass = CLASS, ScriptModule = 7 } }, "must be a string")

		-- Instance-limit consistency.
		rejects("MaxOpenInstances without AllowMultiple",
			{ ["UI.Test"] = { WidgetClass = CLASS, MaxOpenInstances = 5 } }, "disallows multiple")
		accepts("MaxOpenInstances with AllowMultiple",
			{ ["UI.Test"] = { WidgetClass = CLASS, AllowMultiple = true, MaxOpenInstances = 5 } })
		rejects("a negative instance limit",
			{ ["UI.Test"] = { WidgetClass = CLASS, MaxCachedInstances = -1 } }, "negative")

		-- KeepUntilIdle with a non-positive TTL is caching that never caches.
		rejects("KeepUntilIdle with a zero TTL",
			{ ["UI.Test"] = { WidgetClass = CLASS, CachePolicy = "KeepUntilIdle", IdleTimeoutSeconds = 0 } },
			"discards the instance immediately")
		accepts("KeepUntilIdle with a positive TTL",
			{ ["UI.Test"] = { WidgetClass = CLASS, CachePolicy = "KeepUntilIdle", IdleTimeoutSeconds = 10.0 } })
		-- Other policies ignore the field, so a zero there is not an error.
		accepts("a zero TTL under a policy that ignores it",
			{ ["UI.Test"] = { WidgetClass = CLASS, CachePolicy = "KeepPersistent", IdleTimeoutSeconds = 0 } })

		-- Script binding is per class: same class + different modules cannot both win.
		rejects("two modules on one widget class", {
			["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI.A" },
			["UI.Menu"] = { WidgetClass = CLASS, ScriptModule = "UI.B" },
		}, "different")

		-- Same class + same module is a legitimate way to alias one screen.
		accepts("one module shared by two keys", {
			["UI.Test"] = { WidgetClass = CLASS, ScriptModule = "UI.A" },
			["UI.Menu"] = { WidgetClass = CLASS, ScriptModule = "UI.A" },
		})

		-- Two keys on one class with no module at all is unrelated to script binding.
		accepts("two keys sharing a class without modules", {
			["UI.Test"] = { WidgetClass = CLASS },
			["UI.Menu"] = { WidgetClass = CLASS },
		})

		-- An empty table is vacuously valid; Replace uses it to clear the overlay.
		accepts("an empty definitions table", {})

		-- Defaults must mirror FUIWidgetEntry, and the tables must be non-empty.
		if Registry.Defaults.MaxCachedInstances ~= 8 then
			fail("MaxCachedInstances default drifted: %s", tostring(Registry.Defaults.MaxCachedInstances))
		end
		if Registry.Defaults.IdleTimeoutSeconds ~= 60.0 then
			fail("IdleTimeoutSeconds default drifted: %s", tostring(Registry.Defaults.IdleTimeoutSeconds))
		end
		if Registry.Defaults.ScriptModule ~= "" then
			fail("ScriptModule default should be empty")
		end
		if Registry.Layers.FullWindow == nil then fail("FullWindow layer missing") end
		if Registry.Layers.Guide == nil then fail("Guide layer missing") end
		if Registry.Layers.PersistentSystem == nil then fail("PersistentSystem layer missing") end
		if Registry.Layers.Loading == nil then fail("Loading layer missing") end

		UIFrameworkTestResult = "ok"
	)LUA");

	return UIFrameworkLuaTest::RunChunk(*this, *Env, Chunk, TEXT("UIFrameworkLuaRegistryValidateTest"));
}

#endif
