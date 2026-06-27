// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSToggleArchive.h"
#include "FAVSStyle.h"


void FAVSToggleArchive::Construct(const FArguments& InArgs)
{
	OnToggleChanged = InArgs._OnToggleChanged;
	IsChecked = InArgs._IsChecked;

	CheckedBrush   = FAVSStyle::Get().GetBrush("Toggle_ON");
	UncheckedBrush = FAVSStyle::Get().GetBrush("Toggle_OFF");

	ChildSlot
	[
		SNew(SImage)
		.Image_Lambda([this]() -> const FSlateBrush* {
			return IsChecked ? CheckedBrush : UncheckedBrush;
		})
		.OnMouseButtonDown(this, &FAVSToggleArchive::OnMouseClicked)
	];
}

FReply FAVSToggleArchive::OnMouseClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	IsChecked = !IsChecked;

	if (OnToggleChanged.IsBound())
	{
		OnToggleChanged.Execute(IsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	}
	return FReply::Handled();
}
