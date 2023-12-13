// Fill out your copyright notice in the Description page of Project Settings.


#include "SVerticalLoadingWidget.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVerticalLoadingWidget::Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings)
{
	bPlayReverse = Settings.ImageSequenceSettings.bPlayReverse;

	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);

	ConstructLoadingIcon(Settings);

	EVisibility LoadingTextVisibility;

	if (Settings.LoadingText.IsEmpty())
	{
		LoadingTextVisibility = EVisibility::Collapsed;
	}
	else
	{
		LoadingTextVisibility = EVisibility::SelfHitTestInvisible;
	}

	// 如果加载文本在上面	text -> space -> icon
	if (Settings.bLoadingTextTopPosition)
	{
		// .AutoHeight() 表示插槽的高度将根据其子内容自动调整。
		Root.Get().AddSlot()
			.HAlign(Settings.TextAlignment.HorizontalAlignment)
			.VAlign(Settings.TextAlignment.VerticalAlignment)
			.AutoHeight()
			[
				SNew(STextBlock)
					.Visibility(LoadingTextVisibility)
					.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
					.Font(Settings.Appearance.Font)
					.ShadowOffset(Settings.Appearance.ShadowOffset)
					.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
					.Justification(Settings.Appearance.Justification)
					.Text(Settings.LoadingText)
			];

		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D(0.0f, Settings.Space))
			];

		Root.Get().AddSlot()
			.HAlign(Settings.LoadingIconAlignment.HorizontalAlignment)
			.VAlign(Settings.LoadingIconAlignment.VerticalAlignment)
			.AutoHeight()
			[
				LoadingIcon
			];
	}
	
	ChildSlot
	[
		Root
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
