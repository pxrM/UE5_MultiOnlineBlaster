// Copyright TikiStar. All Rights Reserved.

#include "ViewModels/UIViewModelBase.h"

void UUIViewModelBase::Initialize()
{
	if (bInitialized)
	{
		return;
	}
	bInitialized = true;
	BP_OnInitialize();
}

void UUIViewModelBase::Shutdown()
{
	if (!bInitialized)
	{
		return;
	}
	bInitialized = false;
	BP_OnShutdown();
}
