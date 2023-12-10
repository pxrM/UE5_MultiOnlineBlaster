// Fill out your copyright notice in the Description page of Project Settings.


#include "SClassicLayout.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SClassicLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings)
{
	// ʵ����һ�����б����ĵ��Ӳ�����
	// ��SlateUI����У�+ ��������ڽ�������ӵ� SOverlay �����С�����ͨ��ʹ��SOverlay::AddSlot()������ʵ�ֵġ�
	// ��ʹ�� + �����ʱ�����������һ��SOverlay�����Ҳ�������Ҫ��ӵ�SOverlay�еĲ�����
	// ʹ�� + ��������Է���ؽ�����Ӳ�����ӵ� SOverlay �У���ʹ����ʽ���ý�һ�����ò��ֲ�������ʽ��
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBackgroundWidget, Settings.Background)
		];
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
