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
	// Slateͼ��ˢ����һ�����ڱ�ʾͼ��Ķ��󣬿����������û���������ʾͼ��
	TSharedPtr<FDeferredCleanupSlateBrush>ImageBrush;
};
