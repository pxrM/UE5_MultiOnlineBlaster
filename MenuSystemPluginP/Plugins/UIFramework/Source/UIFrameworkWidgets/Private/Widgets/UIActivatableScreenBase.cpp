// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UIActivatableScreenBase.h"
#include "UIViewModelBase.h"
#include "UIViewModelSupport.h"
#include "UIFrameworkCoreModule.h"
#include "Components/Widget.h"

void UUIActivatableScreenBase::NativeConstruct()
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
		UE_LOG(LogUIFramework, Warning, TEXT("UIActivatableScreen: CreateViewModel returned null."));
		return;
	}

	InjectViewModel(ViewModel);
	ViewModel->Initialize();
}

void UUIActivatableScreenBase::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->Shutdown();
	}
	Super::NativeDestruct();
}

TOptional<FUIInputConfig> UUIActivatableScreenBase::GetDesiredInputConfig() const
{
	// Menu screens leave the mouse free; the action router applies this while active.
	return FUIInputConfig(InputMode, EMouseCaptureMode::NoCapture);
}

UUIViewModelBase* UUIActivatableScreenBase::CreateViewModel()
{
	return NewObject<UUIViewModelBase>(this, ViewModelClass);
}

void UUIActivatableScreenBase::InjectViewModel_Implementation(UUIViewModelBase* InViewModel)
{
	UIFrameworkVM::Inject(this, InViewModel);
}
