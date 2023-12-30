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
		// 在顶部添加边框（一个带有边框图和背景图的矩形区域）小部件，然后添加Loading小部件
		Root->AddSlot()
			// 这两行代码设置了布局槽的水平和垂直对齐方式，使其位于画面的顶部中心位置。
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

		// 在底部添加边框小部件，然后添加Tip小部件
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
		// 在顶部添加边框小部件，然后添加Tip小部件
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

		// 在底部添加边框小部件，然后添加Loading小部件
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

	// 构建加载完整文本.
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
