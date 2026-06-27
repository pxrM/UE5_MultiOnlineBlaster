// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FAVSSegment;

class FAVS_API FAVSSegmentDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FAVSSegmentDragDropOp, FDragDropOperation)

	FText SegmentName;
	int32 SegmentID;
	TSharedPtr<FAVSSegment> SourceElement;

	void Init()
	{
		MouseCursor = EMouseCursor::GrabHand;
		TSharedPtr<SWidget> DecoratorWidget = GetDefaultDecorator();

		FDragDropOperation::Construct();
		
	}
	
private:
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return SNew(SBorder)
			.Padding(6)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(STextBlock)
			.Text(SegmentName)
		];
	}
};
