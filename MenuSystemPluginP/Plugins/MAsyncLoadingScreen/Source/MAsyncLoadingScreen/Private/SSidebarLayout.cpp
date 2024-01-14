// Fill out your copyright notice in the Description page of Project Settings.


#include "SSidebarLayout.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SSpacer.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"
#include "SHorizontalLoadingWidget.h"
#include "SVerticalLoadingWidget.h"
#include "STipWidget.h"
#include "SLoadingCompleteText.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSidebarLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FSidebarLayoutSettings& LayoutSettings)
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

	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);
	if (LayoutSettings.bIsLoadingWidgetAtTop)
	{
		// 在顶部添加loading widget
		VerticalBox.Get().AddSlot()
			.AutoHeight()
			.HAlign(LayoutSettings.LoadingWidgetAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.LoadingWidgetAlignment.VerticalAlignment)
			[
				LoadingWidget
			];

		// 在中间添加SSpacer
		VerticalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D(0.0f, LayoutSettings.Space))
			];

		// 在底部添加Tip widget
		VerticalBox.Get().AddSlot()
			.AutoHeight()
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];
	}
	else
	{
		// 在顶部添加loading widget
		VerticalBox.Get().AddSlot()
			.AutoHeight()
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];

		// 在中间添加SSpacer
		VerticalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D(0.0f, LayoutSettings.Space))
			];

		// 在底部添加loading widget
		VerticalBox.Get().AddSlot()
			.AutoHeight()
			.HAlign(LayoutSettings.LoadingWidgetAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.LoadingWidgetAlignment.VerticalAlignment)
			[
				LoadingWidget
			];
	}

	if (LayoutSettings.bIsWidgetAtRight)
	{
		Root.Get().AddSlot()
			.HAlign(HAlign_Right)
			.VAlign(LayoutSettings.BorderVerticalAlignment)
			.Padding(0, 0, LayoutSettings.BorderHorizontalOffset, 0)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(HAlign_Fill)
							.VAlign(LayoutSettings.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SSidebarLayout::GetDPIScale)
									[
										VerticalBox
									]
							]
					]
			];
	}
	else
	{
		Root.Get().AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(LayoutSettings.BorderVerticalAlignment)
			.Padding(LayoutSettings.BorderHorizontalOffset, 0, 0, 0)
			[
				SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.BorderImage(&LayoutSettings.BorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					[
						SNew(SSafeZone)
							.HAlign(HAlign_Fill)
							.VAlign(LayoutSettings.VerticalAlignment)
							.IsTitleSafe(true)
							.Padding(LayoutSettings.BorderPadding)
							[
								SNew(SDPIScaler)
									.DPIScale(this, &SSidebarLayout::GetDPIScale)
									[
										VerticalBox
									]
							]
					]
			];
	}

	if (Settings.bShowLoadingCompleteText)
	{
		Root->AddSlot()
			.HAlign(Settings.LoadingCompleteTextSettings.Alignment.HorizontalAlignment)
			.VAlign(Settings.LoadingCompleteTextSettings.Alignment.VerticalAlignment)
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
