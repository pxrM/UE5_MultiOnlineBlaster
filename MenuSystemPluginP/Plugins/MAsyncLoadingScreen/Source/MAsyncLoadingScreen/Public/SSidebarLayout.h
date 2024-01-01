// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SLoadingScreenLayout.h"

struct FALoadingScreenSettings;
struct FSidebarLayoutSettings;

/**
 * 
 */
class MASYNCLOADINGSCREEN_API SSidebarLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SSidebarLayout)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FSidebarLayoutSettings& LayoutSettings);
};
