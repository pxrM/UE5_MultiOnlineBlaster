// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SLoadingWidget.h"

struct FLoadingWidgetSettings;

/**
 * 
 */
class MASYNCLOADINGSCREEN_API SHorizontalLoadingWidget : public SLoadingWidget
{
public:
	SLATE_BEGIN_ARGS(SHorizontalLoadingWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings);
};
