// Fill out your copyright notice in the Description page of Project Settings.


#include "SCenterLayout.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "SBackgroundWidget.h"
#include "LoadingScreenSettings.h"
#include "SHorizontalLoadingWidget.h"
#include "SVerticalLoadingWidget.h"
#include "STipWidget.h"
#include "SLoadingCompleteText.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCenterLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FCenterLayoutSettings& LayoutSettings)
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

	// 在中心添加加载小部件
	Root->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			LoadingWidget
		];

	if (LayoutSettings.bIsTipAtBottom)
	{
		Root->AddSlot()
			.HAlign(LayoutSettings.BorderHorizontalAlignment)
			.VAlign(VAlign_Bottom)
			.Padding(0, 0, 0, LayoutSettings.BorderVerticalOffset)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SCenterLayout::GetDPIScale)
									[
										SNew(STipWidget, Settings.TipWidget)
									]
							]
					]
			];
	}
	else
	{
		Root->AddSlot()
			.HAlign(LayoutSettings.BorderHorizontalAlignment)
			.VAlign(VAlign_Top)
			.Padding(0, LayoutSettings.BorderVerticalOffset, 0, 0)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
							.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SCenterLayout::GetDPIScale)
									[
										SNew(STipWidget, Settings.TipWidget)
									]
							]
					]
			];
	}

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
