// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SThrobber.h"

class FDeferredCleanupSlateBrush;
struct FLoadingWidgetSettings;
struct FThrobberSettings;

/**
 * Loading Widget base class
 */
class MASYNCLOADINGSCREEN_API SLoadingWidget : public SCompoundWidget
{
public:
	/// <summary>
	/// OnPaint是Slate框架中用于绘制小部件外观的虚函数，它被继承自SWidget类的派生类重载，用于实现自定义的绘制逻辑。
	/// </summary>
	/// <param name="Args"> 包含绘制时所需的各种参数，例如Canvas、时间戳等 </param>
	/// <param name="AllottedGeometry"> 表示在屏幕上分配给小部件的空间信息，包括位置、大小、缩放比例等 </param>
	/// <param name="MyCullingRect"> 表示小部件的矩形区域，用于确定哪些元素需要被绘制 </param>
	/// <param name="OutDrawElements"> 指向元素列表的引用，用于存储需要绘制的元素 </param>
	/// <param name="LayerId"> 用于标识绘制层次结构中的层次ID </param>
	/// <param name="InWidgetStyle"> 当前小部件的样式 </param>
	/// <param name="bParentEnabled"> 表示小部件是否启用 </param>
	/// <returns></returns>
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;


public:
	/// <summary>
	/// 获取一个throbber动画。SThrobber::EAnimation 是一个枚举类型，可能包含了不同的动画类型，比如旋转、闪烁等。
	/// </summary>
	/// <param name="ThrobberSettings"></param>
	/// <returns></returns>
	SThrobber::EAnimation GetThrobberAnimation(const FThrobberSettings& ThrobberSettings) const;

	/// <summary>
	/// 构造 loading icon
	/// </summary>
	/// <param name="Settings"></param>
	void ConstructLoadingIcon(const FLoadingWidgetSettings& Settings);


protected:
	// 获取LoadingWidget的可见性
	EVisibility GetLoadingWidgetVisibility() const;


protected:
	/// <summary>
	/// Loading Icon占位
	/// </summary>
	TSharedRef<SWidget> LoadingIcon = SNullWidget::NullWidget;

	/// <summary>
	/// Image slate brush list
	/// FDeferredCleanupSlateBrush 是一个结构体，用于存储需要延迟清理的 Slate 图像资源。
	/// </summary>
	TArray<TSharedPtr<FDeferredCleanupSlateBrush>> CleanupBrushList;

	/// <summary>
	/// 反向播放图像序列
	/// </summary>
	bool bPlayReverse = false;

	/// <summary>
	/// 当前图片序列的index
	/// </summary>
	mutable int32 ImageIndex = 0;

	/// <summary>
	/// 存储当前总的增量时间
	/// </summary>
	mutable float TotalDeltaTime = 0.0f;

	/// <summary>
	/// 用于指定更新图像的时间间隔（以秒为单位），值越小动画越快。0值将每帧更新图像。
	/// </summary>
	float Interval = 0.05f;
};
