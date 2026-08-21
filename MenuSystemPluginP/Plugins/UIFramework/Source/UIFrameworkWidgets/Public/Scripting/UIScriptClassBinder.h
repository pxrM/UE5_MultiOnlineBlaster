// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UUserWidget;

/**
 * Extension point that lets an optional script-runtime adapter attach a script module
 * to a freshly created widget instance, without UIFrameworkWidgets depending on any
 * specific script runtime.
 *
 * The manager calls this after the widget object and its widget tree exist but before
 * the widget is pushed onto a layer, so a bound script sees populated BindWidget
 * members and still runs ahead of NativeConstruct.
 */
class UIFRAMEWORKWIDGETS_API IUIScriptClassBinder
{
public:
	virtual ~IUIScriptClassBinder() = default;

	/**
	 * Attach ModuleName to Widget.
	 * @param WorldContext used by the adapter to locate its script environment.
	 * @param Widget       the newly constructed instance, never null.
	 * @param ModuleName   runtime-specific module path, e.g. "UI.MainMenu".
	 * @return false if the binding was refused or failed; the widget stays usable.
	 */
	virtual bool BindWidget(UObject* WorldContext, UUserWidget* Widget, const FString& ModuleName) = 0;
};

namespace UIFrameworkScript
{
	/** Install the process-wide binder. Adapters call this from StartupModule. */
	UIFRAMEWORKWIDGETS_API void RegisterClassBinder(IUIScriptClassBinder* Binder);

	/** Remove the binder if it is the one currently installed. */
	UIFRAMEWORKWIDGETS_API void UnregisterClassBinder(IUIScriptClassBinder* Binder);

	/** The installed binder, or null when no script adapter is present. */
	UIFRAMEWORKWIDGETS_API IUIScriptClassBinder* GetClassBinder();

	/**
	 * Bind if an adapter is installed and ModuleName is non-empty. A missing adapter is
	 * logged once per module name so a misconfigured project is visible without the
	 * framework hard-depending on any script runtime.
	 */
	UIFRAMEWORKWIDGETS_API bool BindWidget(
		UObject* WorldContext,
		UUserWidget* Widget,
		const FString& ModuleName);
}
