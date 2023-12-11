// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingWidget.h"


int32 SLoadingWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return int32();
}

SThrobber::EAnimation SLoadingWidget::GetThrobberAnimation(const FThrobberSettings& ThrobberSettings) const
{
	return SThrobber::EAnimation();
}

void SLoadingWidget::ConstructLoadingIcon(const FLoadingWidgetSettings& Settings)
{
}

EVisibility SLoadingWidget::GetLoadingWidgetVisibility() const
{
	return EVisibility();
}
