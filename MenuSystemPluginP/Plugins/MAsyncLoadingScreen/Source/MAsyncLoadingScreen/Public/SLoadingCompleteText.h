// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FLoadingCompleteTextSettings;

/**
 * 
 */
class MASYNCLOADINGSCREEN_API SLoadingCompleteText : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingCompleteText)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

public:
	EVisibility GetLoadingCompleteTextVisibility() const;

	FSlateColor GetLoadingCompleteTextColor() const;

	EActiveTimerReturnType AnimateText(double InCurrentTime, float InDeltaTime);
};
