// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SLoadingScreenLayout.h"

struct FALoadingScreenSettings;
struct FClassicLayoutSettings;

/**
 * 经典布局加载画面
 */
class MASYNCLOADINGSCREEN_API SClassicLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SClassicLayout)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings);
};
