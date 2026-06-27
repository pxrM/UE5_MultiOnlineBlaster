// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class FAVS_API FAVSDraggableHandle : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(FAVSDraggableHandle) {}
		SLATE_EVENT(FPointerEventHandler, OnMouseButtonDown)
		SLATE_EVENT(FOnDragDetected, OnDragDetected)
	SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FPointerEventHandler OnMouseButtonDownHandler;
	FOnDragDetected OnDragDetectedHandler;
	FName DraggableHandle_Element = "DraggableHandle_Element";  
};
