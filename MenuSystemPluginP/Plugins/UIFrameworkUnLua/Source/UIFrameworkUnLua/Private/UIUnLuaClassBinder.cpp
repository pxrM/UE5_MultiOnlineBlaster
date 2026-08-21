// Copyright TikiStar. All Rights Reserved.

#include "UIUnLuaClassBinder.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"
#include "UnLuaInterface.h"
#include "UnLuaManager.h"
#include "UnLuaModule.h"

bool FUIUnLuaClassBinder::BindWidget(UObject* WorldContext, UUserWidget* Widget, const FString& ModuleName)
{
	if (!Widget || ModuleName.IsEmpty())
	{
		return false;
	}

	UClass* WidgetClass = Widget->GetClass();
	if (WidgetClass->ImplementsInterface(UUnLuaInterface::StaticClass()))
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("ScriptModule '%s' ignored for '%s': the class declares its own Lua module through "
				 "IUnLuaInterface. Use either the registry field or the class default, not both."),
			*ModuleName, *WidgetClass->GetName());
		return false;
	}

	IUnLuaModule& UnLuaModule = IUnLuaModule::Get();
	if (!UnLuaModule.IsActive())
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("ScriptModule '%s' not bound for '%s': UnLua is not active."),
			*ModuleName, *WidgetClass->GetName());
		return false;
	}

	UnLua::FLuaEnv* Env = UnLuaModule.GetEnv(WorldContext ? WorldContext : Widget);
	if (!Env)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("ScriptModule '%s' not bound for '%s': no Lua environment for this context."),
			*ModuleName, *WidgetClass->GetName());
		return false;
	}

	UUnLuaManager* Manager = Env->GetManager();
	if (!Manager)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("ScriptModule '%s' not bound for '%s': UnLua manager is unavailable."),
			*ModuleName, *WidgetClass->GetName());
		return false;
	}

	// UnLua binds a module to a UClass. In a non-editor build UUnLuaManager::BindClass
	// returns true immediately for an already-bound class WITHOUT comparing the module
	// name, so a second, different module would silently keep the first one's table
	// while Bind still reported success. Remember the first claim per class and refuse
	// a conflicting one here, where it can be reported.
	//
	// Drop entries whose class is gone first, so a recompiled or collected blueprint
	// class does not accumulate and cannot leave a claim behind for its replacement.
	for (auto It = BoundModuleByClass.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	if (const FString* Claimed = BoundModuleByClass.Find(WidgetClass))
	{
		if (*Claimed != ModuleName)
		{
			UE_LOG(LogUIFramework, Error,
				TEXT("ScriptModule '%s' refused for '%s': the class is already bound to '%s'. "
					 "UnLua binds one module per class, so the new module would be ignored. "
					 "Give the second registry entry its own widget class."),
				*ModuleName, *WidgetClass->GetName(), **Claimed);
			return false;
		}
		// Same module: Bind is idempotent per class and still attaches a per-instance
		// table, so fall through.
	}

	// UnLua logs the specific failure (missing file, syntax error, non-table return).
	if (!Manager->Bind(Widget, *ModuleName))
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("Failed to bind ScriptModule '%s' to '%s'. See the LogUnLua output above."),
			*ModuleName, *WidgetClass->GetName());
		return false;
	}

	BoundModuleByClass.Add(WidgetClass, ModuleName);
	UE_LOG(LogUIFramework, Verbose, TEXT("Bound ScriptModule '%s' to '%s'."),
		*ModuleName, *WidgetClass->GetName());
	return true;
}

void FUIUnLuaClassBinder::Reset()
{
	BoundModuleByClass.Reset();
}
