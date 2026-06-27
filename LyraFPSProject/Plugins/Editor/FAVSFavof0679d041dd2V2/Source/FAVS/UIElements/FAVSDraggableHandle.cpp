// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSDraggableHandle.h"
#include "FAVSStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"

void FAVSDraggableHandle::Construct(const FArguments& InArgs)
{
	OnMouseButtonDownHandler = InArgs._OnMouseButtonDown;
	OnDragDetectedHandler = InArgs._OnDragDetected;

	ChildSlot
	[
		SNew(SImage)
		.Image(FAVSStyle::Get().GetBrush(DraggableHandle_Element))
		.Cursor(EMouseCursor::GrabHand)
	];
	
}

FReply FAVSDraggableHandle::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (OnMouseButtonDownHandler.IsBound())
	{
		return OnMouseButtonDownHandler.Execute(MyGeometry, MouseEvent);
	}
	return FReply::Handled();;
}

FReply FAVSDraggableHandle::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (OnDragDetectedHandler.IsBound())
	{
		return OnDragDetectedHandler.Execute(MyGeometry, MouseEvent);
	}
	return FReply::Unhandled();
}
