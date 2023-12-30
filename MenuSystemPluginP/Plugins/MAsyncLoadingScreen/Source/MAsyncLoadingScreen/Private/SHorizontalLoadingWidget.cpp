// Fill out your copyright notice in the Description page of Project Settings.


#include "SHorizontalLoadingWidget.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SHorizontalLoadingWidget::Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings)
{
	bPlayReverse = Settings.ImageSequenceSettings.bPlayReverse;

	TSharedRef<SHorizontalBox> Root = SNew(SHorizontalBox);

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

	// 如果加载文本在右边	icon -> Space - > text
	if (Settings.bLoadingTextRightPosition)
	{
		// 调用Get()获取Root对象的引用，并调用AddSlot()来添加一个新的插槽（Slot插槽用于定义在容器中放置子控件的方式和样式）
		// .AutoWidth() 表示插槽的宽度将根据其子内容自动调整。
		Root.Get().AddSlot()
			.HAlign(Settings.LoadingIconAlignment.HorizontalAlignment)
			.VAlign(Settings.LoadingIconAlignment.VerticalAlignment)
			.AutoWidth()
			[
				LoadingIcon
			];

		// 添加一定宽度的间隔控件
		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(Settings.Space, 0.0f))
			];

		// 最后在右边添加loading text
		Root.Get().AddSlot()
			.HAlign(Settings.TextAlignment.HorizontalAlignment)
			.VAlign(Settings.TextAlignment.VerticalAlignment)
			.AutoWidth()
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
	}
	else//如果加载文本在左边	text -> Space - > icon
	{
		// 首先在左侧添加加载文本
		Root.Get().AddSlot()
			.HAlign(Settings.TextAlignment.HorizontalAlignment)
			.VAlign(Settings.TextAlignment.VerticalAlignment)
			.AutoWidth()
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

		// 添加一定宽度的间隔控件
		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(Settings.Space, 0.0f))
			];

		// 最后在最右边添加loading icon
		Root.Get().AddSlot()
			.HAlign(Settings.LoadingIconAlignment.HorizontalAlignment)
			.VAlign(Settings.LoadingIconAlignment.VerticalAlignment)
			.AutoWidth()
			[
				LoadingIcon
			];
	}

	// ChildSlot表示当前Widget的子控件插槽，使用[ ]运算符将Root控件作为子控件添加到该插槽中。
	ChildSlot
	[
		Root
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION