// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSSegment.h"
#include "Widgets/SCompoundWidget.h"


class FAVS_API FAVSElement : public SCompoundWidget
{
	
public:
	SLATE_BEGIN_ARGS(FAVSElement){}
		SLATE_ARGUMENT(FAVSAssetData, Asset)
		SLATE_ARGUMENT(int32, SegmentID)
		SLATE_ARGUMENT(FAVSSegment*, Parent)
	SLATE_END_ARGS()
	
	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
private:
	TSharedPtr<SBox> PaddingBox;
	SVerticalBox::FSlot* DragSlot; 
	SVerticalBox::FSlot* Spacer;
	TSharedPtr<SBox> TopSpacer;
	TSharedPtr<SButton> OpenAssetBtn;
	TSharedPtr<SBox> BottomSpacer;
	TSharedPtr<STextBlock> ElementText;
	TSharedPtr<SBox> ElementBox;
protected:
	FName AssetName;
	FAVSAssetData Asset;
	int32 SegmentID = 0;
	FAVSSegment* Parent = nullptr;
	bool AssetIsOpened = false;
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	void HandleFontSizeChanged();
	void Construct(const FArguments& InArgs);
	virtual TSharedPtr<SButton> AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox) = 0;
	virtual FReply OnEyeClicked() = 0; 
	virtual FReply OpenAsset() = 0; 
	virtual FReply CloseAsset() = 0;
	virtual void AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox);
	virtual void AddRemoveBtn(TSharedRef<SHorizontalBox> _HorizontalBox);
	void SetPaddingWhenDragOver(const int32 TopPadding, const int32 BottomPadding);
	bool IsLevel() const;
	FAVSSegment* GetParent();
	void UpdateButtonStyle(const bool _AssetIsOpened);
	void UpdateWidget(const FAVSAssetData& _Asset);
private:
	FReply OnElementDoubleClicked(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	const FSlateBrush* GetIcon();
	FReply RemoveElement();
	FReply OnDragHandleMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
};

