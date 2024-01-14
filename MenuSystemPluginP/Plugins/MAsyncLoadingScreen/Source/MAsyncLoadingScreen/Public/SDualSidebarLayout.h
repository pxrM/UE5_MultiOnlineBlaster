// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SLoadingScreenLayout.h"

struct FALoadingScreenSettings;
struct FDualSidebarLayoutSettings;

/**
 * 
 */
class MASYNCLOADINGSCREEN_API SDualSidebarLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SDualSidebarLayout)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FDualSidebarLayoutSettings& LayoutSettings);
};
