// Fill out your copyright notice in the Description page of Project Settings.


#include "SClassicLayout.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"
#include "STipWidget.h"
#include "SHorizontalLoadingWidget.h"
#include "SVerticalLoadingWidget.h"
#include "SLoadingCompleteText.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SClassicLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings)
{
	// SOverlay是Slate库中的一个容器部件，用于在视觉层次结构中叠加多个子部件。
	// 在SlateUI框架中，+ 运算符用于将部件添加到 SOverlay 容器中。这是通过使用SOverlay::AddSlot()函数来实现的。
	// 当使用 + 运算符时，左操作数是一个SOverlay对象，右操作数是要添加到SOverlay中的部件。
	// 使用 + 运算符可以方便地将多个子部件添加到 SOverlay 中，并使用链式调用进一步设置布局参数和样式。
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBackgroundWidget, Settings.Background)
		];

	// 使用了SNullWidget::NullWidget静态函数创建一个空的SWidget，并将其转换为TSharedRef的形式。
	// 这种写法常用于在程序中暂时不需要使用加载屏幕等UI组件时的占位符，以便在需要时方便地进行替换。
	// 可以根据实际需要，将"SNullWidget::NullWidget"替换为其他的SWidget对象或自定义的UI组件。
	TSharedRef<SWidget> LoadingWidget = SNullWidget::NullWidget;
	if (Settings.LoadingWidget.LoadingWidgetType == ELoadingWidgetType::LWT_Horizontal)
	{
		LoadingWidget = SNew(SHorizontalLoadingWidget, Settings.LoadingWidget);
	}
	else
	{
		LoadingWidget = SNew(SVerticalLoadingWidget, Settings.LoadingWidget);
	}

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);
	// 如果LoadingWidget在左边 Loading -> Space ->  Tip
	if (LayoutSettings.bIsLoadingWidgetAtLeft)
	{
		// 首先在左侧添加LoadingWidget
		HorizontalBox.Get().AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];

		// 添加一定宽度的间隔控件
		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		// 在右边添加tip
		// 表示添加的插槽的宽度将填充所有可用空间，使STipWidget 控件可以水平自适应地拉伸或收缩以适应水平盒子的大小变化。
		HorizontalBox.Get().AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];
	}
	else // tip -> space -> loading
	{
		HorizontalBox.Get().AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];

		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];
	}

	// 设置widget为垂直对齐
	EVerticalAlignment VerticalAlignment;
	if (LayoutSettings.bIsWidgetAtBottom)
	{
		VerticalAlignment = EVerticalAlignment::VAlign_Bottom;
	}
	else
	{
		VerticalAlignment = EVerticalAlignment::VAlign_Top;
	}

	Root->AddSlot()
		.HAlign(LayoutSettings.BorderHorizontalAlignment)
		.VAlign(VerticalAlignment)
		[
			// SBorder控件，用于创建包含边框的矩形区域。它可以用于包裹其他控件，并为它们提供边框样式和背景颜色。
			// .BorderImage() 接受一个图片资源或纹理作为参数，并在控件周围显示该图像作为边框
			// .BorderBackgroundColor表示SBorder控件的边框背景颜色是白色
			SNew(SBorder)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderImage(&LayoutSettings.BorderBackground)
				.BorderBackgroundColor(FLinearColor::White)
				[
					// SSafeZone提供了一种安全区域，可以在各种不同的屏幕分辨率和比例下保持内容的可见性。
					// .IsTitleSafe(true) 表示使用标准的“标题安全区域”，确保内容不会被裁剪或遮挡。
					SNew(SSafeZone)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.IsTitleSafe(true)
						.Padding(LayoutSettings.BorderPadding)
						[
							// 用于根据设备的像素密度（DPI）自动缩放其子控件的大小。
							// 在不同的设备上，像素密度可以有所不同，这意味着相同的尺寸在不同的设备上可能会显示得更大或更小。
							// SDPIScaler 控件接受一个子控件，并将该子控件的大小乘以当前设备的像素密度因子，从而实现自动缩放。
							SNew(SDPIScaler)
								.DPIScale(this, &SClassicLayout::GetDPIScale)
								[
									HorizontalBox
								]
						]
				]
		];

	// 如果启用，构建加载完整文本
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
