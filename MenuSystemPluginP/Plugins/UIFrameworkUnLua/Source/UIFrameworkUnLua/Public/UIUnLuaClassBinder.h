// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Scripting/UIScriptClassBinder.h"
#include "UObject/WeakObjectPtr.h"

/**
 * Binds UIFramework widget instances to Lua modules through UnLua, so a registry
 * entry can name a script module instead of requiring the widget blueprint to derive
 * from UUIUnLuaScreenBase and carry a static LuaModuleName.
 *
 * UnLua binds a module to a UClass and then attaches a per-instance table, so the
 * same widget class cannot serve two different modules. The registry rejects that
 * combination up front (see ValidateUIScriptModuleUniqueness); this class is the
 * backstop for the cases the registry cannot see, because in a non-editor build
 * UnLua accepts the second bind and silently keeps the first module.
 *
 * A widget class that already implements IUnLuaInterface has its own static module.
 * Binding a different one on top would fight UnLua's own resolution, so this binder
 * refuses and logs instead.
 */
class FUIUnLuaClassBinder final : public IUIScriptClassBinder
{
public:
	virtual bool BindWidget(UObject* WorldContext, UUserWidget* Widget, const FString& ModuleName) override;

	/** Forget every recorded class->module claim. For module shutdown and tests. */
	void Reset();

private:
	/**
	 * First module bound per widget class. Weak keys because a blueprint class can be
	 * recompiled or garbage collected; a stale raw UClass* would either leak a false
	 * conflict onto a reused address or dangle.
	 */
	TMap<TWeakObjectPtr<UClass>, FString> BoundModuleByClass;
};
