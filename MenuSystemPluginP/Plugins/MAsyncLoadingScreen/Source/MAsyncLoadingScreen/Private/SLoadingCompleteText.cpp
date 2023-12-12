// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingCompleteText.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoadingCompleteText::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}

EVisibility SLoadingCompleteText::GetLoadingCompleteTextVisibility() const
{
	return EVisibility();
}

FSlateColor SLoadingCompleteText::GetLoadingCompleteTextColor() const
{
	return FSlateColor();
}

EActiveTimerReturnType SLoadingCompleteText::AnimateText(double InCurrentTime, float InDeltaTime)
{
	return EActiveTimerReturnType();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
