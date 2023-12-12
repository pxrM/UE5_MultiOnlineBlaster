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
	// loadingС��������һ����ʱ�������л���ʾ�ļ���ͼ��

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
					// ͼ����Դ�Ĵ�С������ Scale ���ű������м��㡣GetSurfaceHeight()���ڻ�ȡ Slate ͼ����Դ�ı���߶ȣ�������������
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
			// ���������û��ͼ���򴴽�һ�������
			// SSpacer ��UE�е�һ��С���������ڴ����հ׵�ռλ���������Ը���ָ���ĳߴ���������С��
			LoadingIcon = SNew(SSpacer).Size(FVector2D::ZeroVector);
		}
	}
	else if (Settings.LoadingIconType == ELoadingIconType::LIT_CircularThrobber)
	{
		// SCircularThrobber ��UE�е�С������������ʾһ����ת�Ķ�����ͨ����������ָʾ�����߽���ָʾ����
		// ���С��������ͨ�����ò�ͬ�����������ƶ�������ۺ���Ϊ��������ת�ٶȡ���ɫ�ȡ�
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

	// ����loading icon��Ⱦ�任����Ⱦ�任�����ĵ�
	// FSlateRenderTransform ��һ�����ڱ�ʾ Slate �ؼ���Ⱦ�任�Ľṹ�壬���а��������ź�ƽ�Ƶ���Ϣ��
	// FScale2D(Settings.TransformScale) ��ʾ��Ⱦ�任�����Ų��֣�Settings.TransformTranslation ��ʾ��Ⱦ�任��ƽ�Ʋ��֡�
	LoadingIcon.Get().SetRenderTransform(FSlateRenderTransform(FScale2D(Settings.TransformScale), Settings.TransformTranslation));
	LoadingIcon.Get().SetRenderTransformPivot(Settings.TransformPivot);

	if (Settings.bHideLoadingWidgetWhenCompletes)
	{
		// ���ؼ��Ŀɼ��������� GetLoadingWidgetVisibility() �������а󶨣��Ա��ڼ������ʱ�Զ����¿ؼ��Ŀɼ���״̬��
		// TAttribute<EVisibility>::Create()���ڴ���һ��TAttribute<EVisibility> ���󣬸ö����������ÿؼ��Ŀɼ��ԡ�
		//	 ��ĳЩ����£��ؼ��Ŀɼ�����Ҫ����ĳЩ��������̬�����ã���������û����������ݼ���״̬�ȡ�
		//	 ��ʱ������ʹ�� TAttribute<EVisibility> ��������ʾ�ؼ��Ŀɼ������ԣ���������ĳ��������ί�н��а󶨣��Ա����ض����¼�����ʱ�Զ����¿ؼ��Ŀɼ���״̬��
		// TAttribute<EVisibility>::FGetter::CreateRaw()���ڴ���һ������ָ����󣬸ú���ָ��������ڻ�ȡ�ؼ��Ŀɼ���״̬��
		// FGetter ��һ�������������ڻ�ȡĳ�����Ե�ֵ���� Slate �У�FGetter ͨ�����������Զ����һ����Ա�����ڻ�ȡ�����Եĵ�ǰֵ��
		SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateRaw(this, &SLoadingWidget::GetLoadingWidgetVisibility)));
	}
}

EVisibility SLoadingWidget::GetLoadingWidgetVisibility() const
{
	return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Hidden : EVisibility::Visible;
}
