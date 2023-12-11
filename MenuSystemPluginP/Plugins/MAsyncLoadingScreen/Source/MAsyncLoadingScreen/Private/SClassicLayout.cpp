// Fill out your copyright notice in the Description page of Project Settings.


#include "SClassicLayout.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"
#include "STipWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SClassicLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings)
{
	// 实现了一个具有背景的叠加层容器
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

	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
