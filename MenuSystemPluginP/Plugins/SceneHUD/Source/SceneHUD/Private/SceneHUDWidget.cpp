// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneHUDWidget.h"

#include "Engine/Canvas.h"

void FSceneHUDWidget::Create(const FSceneHUDWidgetData& InData)
{
	Name = InData.Name;
	OriginPosition = InData.Position;
	Pivot = InData.Pivot;
	Padding = InData.Padding;
	OriginSize = InData.Size;
	bSizeToContent = InData.bSizeToContent;
}

void FSceneHUDWidget::UpdateLayout()
{
}

void FSceneHUDWidget::GetAllValidHandleID(TArray<int32>& OutHandles)
{
	if(HandleID > 0)
	{
		OutHandles.Add(HandleID);
	}
}

FSceneHUDWidget* FSceneHUDWidget::FindWidget(const FName& InName)
{
	return Name.IsEqual(InName) ? this : nullptr;
}

FVector2D FSceneHUDWidget::GetLayoutSize() const
{
	return bSizeToContent ? GetContentSize() : OriginSize;
}

FVector2D FSceneHUDWidget::GetLayoutSizeWithPadding() const
{
	return GetLayoutSize() + FVector2D(Padding.Left + Padding.Right, Padding.Top + Padding.Bottom);
}

const FName& FSceneHUDWidget::GetName() const
{
	return Name;
}

bool FSceneHUDWidget::GetVisible() const
{
	return Visible;
}

void FSceneHUDWidget::SetVisible(const bool InVisible)
{
	Visible = InVisible;
}

const FMargin& FSceneHUDWidget::GetPadding()
{
	return Padding;
}

void FSceneHUDWidget::SetPadding(const FMargin& InPadding)
{
	Padding = InPadding;
}

void FSceneHUDWidget::SetHandleID(const int32 InHandleID)
{
	HandleID = InHandleID;
}

int32 FSceneHUDWidget::GetHandleID() const
{
	return HandleID;
}

/*
 * InScale：表示一个缩放因子，用于调整位置计算结果的大小。
 * InPosition：表示一个额外的位置偏移，可以在计算后再次加到结果中。
 * 
 */
FVector2D FSceneHUDWidget::GetDrawPosition(const float InScale, const FVector2D& InPosition) const
{
	return InScale * (LayoutPosition + AnimPosition + OriginPosition +
		FVector2D{Padding.Left, Padding.Top} - GetContentSize() * Pivot * Scale) + InPosition;
}

void FSceneHUDWidget::DrawDebug(UCanvas* InCanvas, const FVector2D& InPosition, const float InScale,
                                const float LayoutThickness, const float ContentThickness,
                                const FLinearColor& LayoutColor, const FLinearColor& ContentColor) const
{
	InCanvas->K2_DrawBox(InScale * LayoutPosition + InPosition, GetLayoutSizeWithPadding() * InScale, 0,
	                     FLinearColor::Gray);
	InCanvas->K2_DrawBox(GetDrawPosition(InScale, InPosition), GetContentSize() * Scale * InScale, 1,
	                     FLinearColor::Green);
}
