// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UUserWidget;
class UUIViewModelBase;

/**
 * Shared view-model wiring so any widget base — plain UUserWidget or a CommonUI
 * activatable screen — can host a view model without duplicating the MVVM injection
 * detail. See Docs/DESIGN.md section 2.
 */
namespace UIFrameworkVM
{
	/**
	 * Inject a view model instance into a widget's MVVM bindings, matching it to the
	 * declared viewmodel context by class. Returns false if the widget has no MVVM
	 * view (no viewmodel/binding authored in the WBP).
	 */
	UIFRAMEWORKWIDGETS_API bool Inject(UUserWidget* Widget, UUIViewModelBase* ViewModel);
}
