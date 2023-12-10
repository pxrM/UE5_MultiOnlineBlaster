// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

struct FBackgroundSettings;
class FDeferredCleanupSlateBrush;

/**
 * Background widget
 */
class MASYNCLOADINGSCREEN_API SBackgroundWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBackgroundWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FBackgroundSettings& Settings);


private:
	// Slate图像刷子是一种用于表示图像的对象，可以用于在用户界面中显示图像
	TSharedPtr<FDeferredCleanupSlateBrush>ImageBrush;
};
