// Copyright TikiStar. All Rights Reserved.

#include "UIViewModelSupport.h"
#include "UIViewModelBase.h"
#include "UIFrameworkCoreModule.h"
#include "Blueprint/UserWidget.h"
#include "View/MVVMView.h"

bool UIFrameworkVM::Inject(UUserWidget* Widget, UUIViewModelBase* ViewModel)
{
	if (!Widget || !ViewModel)
	{
		return false;
	}

	// The MVVM view extension exists once the widget has any binding. Match the
	// injected instance to the declared viewmodel context by class.
	if (UMVVMView* View = Cast<UMVVMView>(Widget->GetExtension(UMVVMView::StaticClass())))
	{
		View->SetViewModelByClass(ViewModel);
		return true;
	}

	UE_LOG(LogUIFramework, Warning, TEXT("UIFrameworkVM::Inject: no MVVM view. Add a viewmodel + binding in the WBP, or override injection."));
	return false;
}
