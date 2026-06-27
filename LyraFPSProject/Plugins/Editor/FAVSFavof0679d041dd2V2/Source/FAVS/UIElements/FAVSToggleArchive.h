// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

class FAVS_API FAVSToggleArchive : public SCompoundWidget
{
	public:
		SLATE_BEGIN_ARGS(FAVSToggleArchive){}
			SLATE_EVENT(FOnCheckStateChanged, OnToggleChanged)
		    SLATE_ARGUMENT(bool, IsChecked)
	SLATE_END_ARGS()

	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
private:
	FOnCheckStateChanged OnToggleChanged;
	bool IsChecked = false;

	const FSlateBrush* CheckedBrush = nullptr;
	const FSlateBrush* UncheckedBrush = nullptr;
		
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	void Construct(const FArguments& InArgs);

private:
	FReply OnMouseClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	
};
