// Copyright TikiStar. All Rights Reserved.

#include "UIViewModelWidgetBase.h"
#include "UIViewModelBase.h"
#include "UIViewModelSupport.h"
#include "UIFrameworkCoreModule.h"

void UUIViewModelWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!ViewModelClass)
	{
		return;
	}

	ViewModel = CreateViewModel();
	if (!ViewModel)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("UIViewModelWidget: CreateViewModel returned null."));
		return;
	}

	ViewModel->Initialize();
	InjectViewModel(ViewModel);
}

void UUIViewModelWidgetBase::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->Shutdown();
		ViewModel = nullptr;
	}
	Super::NativeDestruct();
}

UUIViewModelBase* UUIViewModelWidgetBase::CreateViewModel()
{
	return NewObject<UUIViewModelBase>(this, ViewModelClass);
}

void UUIViewModelWidgetBase::InjectViewModel_Implementation(UUIViewModelBase* InViewModel)
{
	UIFrameworkVM::Inject(this, InViewModel);
}
