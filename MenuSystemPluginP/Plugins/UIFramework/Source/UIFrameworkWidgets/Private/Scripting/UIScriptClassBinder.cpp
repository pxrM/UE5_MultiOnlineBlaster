// Copyright TikiStar. All Rights Reserved.

#include "UIScriptClassBinder.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"

namespace UIFrameworkScript
{
	namespace
	{
		IUIScriptClassBinder* GClassBinder = nullptr;

		/** Module names already warned about, so a missing adapter logs once each. */
		TSet<FString>& GetWarnedModules()
		{
			static TSet<FString> Warned;
			return Warned;
		}
	}

	void RegisterClassBinder(IUIScriptClassBinder* Binder)
	{
		if (GClassBinder && Binder && GClassBinder != Binder)
		{
			UE_LOG(LogUIFramework, Warning,
				TEXT("UIFrameworkScript: replacing an already registered script class binder. "
					 "Only one script runtime adapter can drive widget binding."));
		}
		GClassBinder = Binder;
		GetWarnedModules().Reset();
	}

	void UnregisterClassBinder(IUIScriptClassBinder* Binder)
	{
		if (GClassBinder == Binder)
		{
			GClassBinder = nullptr;
		}
	}

	IUIScriptClassBinder* GetClassBinder()
	{
		return GClassBinder;
	}

	bool BindWidget(UObject* WorldContext, UUserWidget* Widget, const FString& ModuleName)
	{
		if (ModuleName.IsEmpty() || !Widget)
		{
			return false;
		}

		if (!GClassBinder)
		{
			bool bAlreadyWarned = false;
			GetWarnedModules().Add(ModuleName, &bAlreadyWarned);
			if (!bAlreadyWarned)
			{
				UE_LOG(LogUIFramework, Warning,
					TEXT("Registry entry requests ScriptModule '%s' but no script adapter is installed. "
						 "Enable a script adapter plugin (for example UIFrameworkUnLua) or clear the field."),
					*ModuleName);
			}
			return false;
		}

		return GClassBinder->BindWidget(WorldContext, Widget, ModuleName);
	}
}
