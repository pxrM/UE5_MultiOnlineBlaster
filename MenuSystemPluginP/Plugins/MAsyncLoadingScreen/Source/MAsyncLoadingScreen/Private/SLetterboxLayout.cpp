// Fill out your copyright notice in the Description page of Project Settings.


#include "SLetterboxLayout.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"
#include "STipWidget.h"
#include "SHorizontalLoadingWidget.h"
#include "SVerticalLoadingWidget.h"
#include "SLoadingCompleteText.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLetterboxLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FLetterboxLayoutSettings& LayoutSettings)
{
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBackgroundWidget, Settings.Background)
		];

	TSharedRef<SWidget> LoadingWidget = SNullWidget::NullWidget;
	if (Settings.LoadingWidget.LoadingWidgetType == ELoadingWidgetType::LWT_Horizontal)
	{
		LoadingWidget = SNew(SHorizontalLoadingWidget, Settings.LoadingWidget);
	}
	else
	{
		LoadingWidget = SNew(SVerticalLoadingWidget, Settings.LoadingWidget);
	}

	if (LayoutSettings.bIsLoadingWidgetAtTip)
	{
		// �ڶ�����ӱ߿�һ�����б߿�ͼ�ͱ���ͼ�ľ�������С������Ȼ�����LoadingС����
		Root->AddSlot()
			// �����д��������˲��ֲ۵�ˮƽ�ʹ�ֱ���뷽ʽ��ʹ��λ�ڻ���Ķ�������λ�á�
			.HAlign(LayoutSettings.TopBorderHorizontalAlignment)
			.VAlign(VAlign_Top)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.TopBorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.LoadingWidgetAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.LoadingWidgetAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.TopBorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SLetterboxLayout::GetDPIScale)
									[
										LoadingWidget
									]
							]
					]
			];

		// �ڵײ���ӱ߿�С������Ȼ�����TipС����
		Root->AddSlot()
			.HAlign(LayoutSettings.BottomBorderHorizontalAlignment)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BottomBorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BottomBorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SLetterboxLayout::GetDPIScale)
									[
										SNew(STipWidget, Settings.TipWidget)
									]
							]
					]
			];
	}
	else
	{
		// �ڶ�����ӱ߿�С������Ȼ�����TipС����
		Root->AddSlot()
			.HAlign(LayoutSettings.TopBorderHorizontalAlignment)
			.VAlign(VAlign_Top)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.TopBorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.TopBorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SLetterboxLayout::GetDPIScale)
									[
										SNew(STipWidget, Settings.TipWidget)
									]
							]
					]
			];

		// �ڵײ���ӱ߿�С������Ȼ�����LoadingС����
		Root->AddSlot()
			.HAlign(LayoutSettings.BottomBorderHorizontalAlignment)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BottomBorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.LoadingWidgetAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.LoadingWidgetAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BottomBorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SLetterboxLayout::GetDPIScale)
									[
										LoadingWidget
									]
							]
					]
			];
	}

	// �������������ı�.
	if (Settings.bShowLoadingCompleteText)
	{
		Root->AddSlot()
			.VAlign(Settings.LoadingCompleteTextSettings.Alignment.VerticalAlignment)
			.HAlign(Settings.LoadingCompleteTextSettings.Alignment.HorizontalAlignment)
			.Padding(Settings.LoadingCompleteTextSettings.Padding)
			[
				SNew(SLoadingCompleteText, Settings.LoadingCompleteTextSettings)
			];
	}

	
	ChildSlot
	[
		Root
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
