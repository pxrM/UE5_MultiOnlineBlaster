// Copyright TikiStar. All Rights Reserved.

#include "UIViewModelWidgetBase.h"
#include "UIViewModelBase.h"
#include "UIViewModelSupport.h"
#include "UIFrameworkCoreModule.h"

void UUIViewModelWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ViewModelClass)
	{
		return;
	}

	if (!ViewModel)
	{
		ViewModel = CreateViewModel();
	}
	if (!ViewModel)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("UIViewModelWidget: CreateViewModel returned null."));
		return;
	}

	InjectViewModel(ViewModel);
	ViewModel->Initialize();
}

void UUIViewModelWidgetBase::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->Shutdown();
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
