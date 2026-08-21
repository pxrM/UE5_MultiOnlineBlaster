// Copyright TikiStar. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "LuaEnv.h"
#include "Misc/AutomationTest.h"
#include "UnLuaModule.h"
#include "lua.hpp"

namespace UIFrameworkLuaTest
{
	/**
	 * UnLua only activates itself for a running game or PIE (see AUTO_UNLUA_STARTUP in
	 * UnLuaModule::StartupModule), so an editor automation run has to switch it on and
	 * put it back afterwards.
	 */
	struct FScopedUnLuaActivation
	{
		FScopedUnLuaActivation()
			: bWasActive(IUnLuaModule::Get().IsActive())
		{
			if (!bWasActive)
			{
				IUnLuaModule::Get().SetActive(true);
			}
		}

		~FScopedUnLuaActivation()
		{
			if (!bWasActive)
			{
				IUnLuaModule::Get().SetActive(false);
			}
		}

		FScopedUnLuaActivation(const FScopedUnLuaActivation&) = delete;
		FScopedUnLuaActivation& operator=(const FScopedUnLuaActivation&) = delete;

		const bool bWasActive;
	};

	/** Read a Lua global as a string, or an unset optional if it is not one. */
	inline TOptional<FString> GetGlobalString(lua_State* L, const char* Name)
	{
		lua_getglobal(L, Name);
		TOptional<FString> Result;
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			Result = UTF8_TO_TCHAR(lua_tostring(L, -1));
		}
		lua_pop(L, 1);
		return Result;
	}

	/** Clear the globals a test chunk reports through, so results cannot leak between tests. */
	inline void ResetTestGlobals(lua_State* L)
	{
		lua_pushnil(L);
		lua_setglobal(L, "UIFrameworkTestError");
		lua_pushnil(L);
		lua_setglobal(L, "UIFrameworkTestResult");
	}

	/**
	 * Run a test chunk that reports failure by setting UIFrameworkTestError and success
	 * by setting UIFrameworkTestResult to "ok". Surfacing the Lua-side message beats a
	 * bare "chunk failed", which is why the chunks use a global instead of a raw error.
	 *
	 * @return true when the chunk completed and reported success.
	 */
	inline bool RunChunk(
		FAutomationTestBase& Test,
		UnLua::FLuaEnv& Env,
		const FString& Chunk,
		const TCHAR* ChunkName)
	{
		lua_State* L = Env.GetMainState();
		ResetTestGlobals(L);

		const bool bRan = Env.DoString(Chunk, ChunkName);

		const TOptional<FString> LuaError = GetGlobalString(L, "UIFrameworkTestError");
		if (LuaError.IsSet() && !LuaError->IsEmpty())
		{
			Test.AddError(FString::Printf(TEXT("Lua assertion failed: %s"), **LuaError));
			return false;
		}
		if (!bRan)
		{
			Test.AddError(FString::Printf(
				TEXT("%s failed to run and reported no assertion; see LogUnLua output."), ChunkName));
			return false;
		}

		const TOptional<FString> Result = GetGlobalString(L, "UIFrameworkTestResult");
		if (!Result.IsSet() || *Result != TEXT("ok"))
		{
			Test.AddError(FString::Printf(TEXT("%s did not run to completion."), ChunkName));
			return false;
		}
		return true;
	}
}

#endif
