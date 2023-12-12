// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingWidget.h"
#include "Widgets/Images/SImage.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Layout/SSpacer.h"
#include "Engine/Texture2D.h"
#include "MoviePlayer.h"
#include "LoadingScreenSettings.h"


int32 SLoadingWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// loading小部件按照一定的时间间隔来切换显示的加载图标

	TotalDeltaTime += Args.GetDeltaTime();

	if (TotalDeltaTime >= Interval)
	{
		if (CleanupBrushList.Num() > 1)
		{
			if (bPlayReverse)
			{
				ImageIndex--;
			}
			else
			{
				ImageIndex++;
			}

			if (ImageIndex >= CleanupBrushList.Num())
			{
				ImageIndex = 0;
			}
			else if (ImageIndex < 0)
			{
				ImageIndex = CleanupBrushList.Num() - 1;
			}

			StaticCastSharedRef<SImage>(LoadingIcon)->SetImage(CleanupBrushList[ImageIndex].IsValid() ? CleanupBrushList[ImageIndex]->GetSlateBrush() : nullptr);
		}

		TotalDeltaTime = 0.0f;
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

SThrobber::EAnimation SLoadingWidget::GetThrobberAnimation(const FThrobberSettings& ThrobberSettings) const
{
	const int32 AnimationParams = (ThrobberSettings.bAnimateVertically ? SThrobber::Vertical : 0) |
		(ThrobberSettings.bAnimateHorizontally ? SThrobber::Horizontal : 0) |
		(ThrobberSettings.bAnimateOpacity ? SThrobber::Opacity : 0);

	return static_cast<SThrobber::EAnimation>(AnimationParams);
}

void SLoadingWidget::ConstructLoadingIcon(const FLoadingWidgetSettings& Settings)
{
	if (Settings.LoadingIconType == ELoadingIconType::LIT_ImageSequence)
	{
		// Loading Widget is image sequence
		if (Settings.ImageSequenceSettings.Images.Num() > 0)
		{
			CleanupBrushList.Empty();
			ImageIndex = 0;

			FVector2D Scale = Settings.ImageSequenceSettings.Scale;

			for (auto Image : Settings.ImageSequenceSettings.Images)
			{
				if (Image)
				{
					// 图像资源的大小，根据 Scale 缩放比例进行计算。GetSurfaceHeight()用于获取 Slate 图像资源的表面高度（即像素数）。
					FVector2D ImageSize = FVector2D(Image->GetSurfaceWidth() * Scale.X, Image->GetSurfaceHeight() * Scale.Y);
					CleanupBrushList.Add(FDeferredCleanupSlateBrush::CreateBrush(Image, ImageSize));
				}
			}

			// Create Image slate widget
			LoadingIcon = SNew(SImage)
				.Image(CleanupBrushList[ImageIndex]->GetSlateBrush());

			// Update play animation interval
			Interval = Settings.ImageSequenceSettings.Interval;
		}
		else
		{
			// 如果数组中没有图像，则创建一个间隔符
			// SSpacer 是UE中的一个小部件，用于创建空白的占位符，它可以根据指定的尺寸来调整大小。
			LoadingIcon = SNew(SSpacer).Size(FVector2D::ZeroVector);
		}
	}
	else if (Settings.LoadingIconType == ELoadingIconType::LIT_CircularThrobber)
	{
		// SCircularThrobber 是UE中的小部件，用于显示一个旋转的动画，通常用作加载指示器或者进度指示器。
		// 这个小部件可以通过设置不同的属性来控制动画的外观和行为，比如旋转速度、颜色等。
		LoadingIcon = SNew(SCircularThrobber)
			.NumPieces(Settings.CircularThrobberSettings.NumberOfPieces)
			.Period(Settings.CircularThrobberSettings.Period)
			.Radius(Settings.CircularThrobberSettings.Radius)
			.PieceImage(&Settings.CircularThrobberSettings.Image);
	}
	else
	{
		LoadingIcon = SNew(SThrobber)
			.NumPieces(Settings.ThrobberSettings.NumberOfPieces)
			.Animate(GetThrobberAnimation(Settings.ThrobberSettings))
			.PieceImage(&Settings.ThrobberSettings.Image);
	}

	// 设置loading icon渲染变换和渲染变换的中心点
	// FSlateRenderTransform 是一个用于表示 Slate 控件渲染变换的结构体，其中包含了缩放和平移的信息。
	// FScale2D(Settings.TransformScale) 表示渲染变换的缩放部分，Settings.TransformTranslation 表示渲染变换的平移部分。
	LoadingIcon.Get().SetRenderTransform(FSlateRenderTransform(FScale2D(Settings.TransformScale), Settings.TransformTranslation));
	LoadingIcon.Get().SetRenderTransformPivot(Settings.TransformPivot);

	if (Settings.bHideLoadingWidgetWhenCompletes)
	{
		// 将控件的可见性属性与 GetLoadingWidgetVisibility() 函数进行绑定，以便在加载完成时自动更新控件的可见性状态。
		// TAttribute<EVisibility>::Create()用于创建一个TAttribute<EVisibility> 对象，该对象将用于设置控件的可见性。
		//	 在某些情况下，控件的可见性需要根据某些条件来动态地设置，例如根据用户操作、数据加载状态等。
		//	 此时，可以使用 TAttribute<EVisibility> 对象来表示控件的可见性属性，并将其与某个函数或委托进行绑定，以便在特定的事件发生时自动更新控件的可见性状态。
		// TAttribute<EVisibility>::FGetter::CreateRaw()用于创建一个函数指针对象，该函数指针对象用于获取控件的可见性状态。
		// FGetter 是一个函数对象，用于获取某个属性的值。在 Slate 中，FGetter 通常被用作属性对象的一个成员，用于获取该属性的当前值。
		SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateRaw(this, &SLoadingWidget::GetLoadingWidgetVisibility)));
	}
}

EVisibility SLoadingWidget::GetLoadingWidgetVisibility() const
{
	return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Hidden : EVisibility::Visible;
}
