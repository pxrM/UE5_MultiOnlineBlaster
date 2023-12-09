// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

struct FTipSettings;

/**
 * Tip widget
 */
class MASYNCLOADINGSCREEN_API STipWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STipWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FTipSettings& Settings);
};
