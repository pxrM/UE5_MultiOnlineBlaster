// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FAVS_API FAVSElementDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FAVSElementDragDropOp, FDragDropOperation)

	FName AssetName;
	FAVSAssetData Asset;
	int32 SegmentID;
	TSharedPtr<FAVSElement> SourceElement;

	void Init()
	{
		MouseCursor = EMouseCursor::GrabHand;
		TSharedPtr<SWidget> DecoratorWidget = GetDefaultDecorator();
		FDragDropOperation::Construct();
	}
	
private:
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDefaultDecorator called: %s"), *AssetName.ToString());
		return SNew(SBorder)
			.Padding(6)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(STextBlock)
			.Text(FText::FromName(AssetName))
		];
	}
};