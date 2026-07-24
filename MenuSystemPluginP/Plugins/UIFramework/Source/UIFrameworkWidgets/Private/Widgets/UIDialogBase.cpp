// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UIDialogBase.h"
#include "Widgets/UIButtonBase.h"
#include "UIManagerSubsystem.h"
#include "UILayerSubsystem.h"
#include "Components/TextBlock.h"

void UUIDialogBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UUIDialogBase::HandleConfirm);
		ConfirmButton->OnClicked.AddDynamic(this, &UUIDialogBase::HandleConfirm);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveDynamic(this, &UUIDialogBase::HandleCancel);
		CancelButton->OnClicked.AddDynamic(this, &UUIDialogBase::HandleCancel);
	}
}

void UUIDialogBase::Setup(const FText& InTitle, const FText& InBody)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
	}
	if (BodyText)
	{
		BodyText->SetText(InBody);
	}
}

void UUIDialogBase::HandleConfirm()
{
	OnConfirmed.Broadcast();
	Close();
}

void UUIDialogBase::HandleCancel()
{
	OnCancelled.Broadcast();
	Close();
}

void UUIDialogBase::Close()
{
	if (UUIManagerSubsystem* Manager = UUIManagerSubsystem::Get(this))
	{
		if (Manager->CloseWidget(this))
		{
			return;
		}
	}

	if (UUILayerSubsystem* Layers = UUILayerSubsystem::Get(this))
	{
		// Direct layer usage is untracked; remove this exact instance instead of
		// accidentally popping a different modal that was pushed above it.
		if (!Layers->RemoveWidget(EUILayer::PopupWindow, this))
		{
			RemoveFromParent();
		}
	}
	else
	{
		// Fallback if the subsystem is unavailable (e.g. dialog added outside the layer system).
		RemoveFromParent();
	}
}
