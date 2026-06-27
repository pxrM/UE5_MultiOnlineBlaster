// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSSegment.h"
#include "FAVS/Data/FAVSDataAsset.h"
#include "Widgets/SCompoundWidget.h"


class FAVS_API FAVSHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(FAVSHeader){}
		SLATE_ARGUMENT(int32, SegmentID)
		SLATE_ARGUMENT(FAVSSegment*, Parent)
	SLATE_END_ARGS()

	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
private:
	int32 SegmentID;
	FAVSSegment* Parent;
	FLinearColor PickedColor;

	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	void Construct(const FArguments& InArgs);
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	void SetToggleBtnStyle();

private:
	void HandleFontSizeChanged();
	TSharedPtr<SEditableText> HeaderText;
	TSharedPtr<SButton> ToggleButton;
	const FButtonStyle* GetCurrentStyle();
	FReply RemoveSegment();
	TSharedRef<SWidget> BuildContextMenu();
	void OnArchiveClicked();
	void PickHeaderColor();
	void OnColorPicked(FLinearColor PickedColor);
	void OnColorNotPicked(FLinearColor PickedColor);
	void OnColorPickerClosed(const TSharedRef<SWindow>& Window);
	void OnUnarchiveClicked();
	FReply ToggleCollapse();
	void OnHeaderChanged(const FText& Text, ETextCommit::Type Arg);
	FAVSSegment* GetParent() const;
	FAVSSegmentData* GetSegmentData();
	FReply OnDragHandleMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
};
