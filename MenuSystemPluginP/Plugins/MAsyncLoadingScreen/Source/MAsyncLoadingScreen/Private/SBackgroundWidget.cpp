// Fill out your copyright notice in the Description page of Project Settings.


#include "SBackgroundWidget.h"
#include "SlateOptMacros.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "AsyncLoadingScreenFuncLibrary.h"
#include "LoadingScreenSettings.h"
#include "MAsyncLoadingScreen.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBackgroundWidget::Construct(const FArguments& InArgs, const FBackgroundSettings& Settings)
{
	if (Settings.Images.Num() > 0)
	{
		int32 ImageIndex = FMath::RandRange(0, Settings.Images.Num() - 1);

		if (Settings.bSetDisplayBackgroundManually)
		{
			if (Settings.Images.IsValidIndex(UAsyncLoadingScreenFuncLibrary::GetDisplayBackgroundIndex()))
			{
				ImageIndex = UAsyncLoadingScreenFuncLibrary::GetDisplayBackgroundIndex();
			}
		}

		UTexture2D* LoadingImage = nullptr;

		// 如果启用了IsPreloadBackgroundImagesEnabled，则从图像数组加载
		FMAsyncLoadingScreenModule& LoadingScreenModule = FMAsyncLoadingScreenModule::Get();
		if (LoadingScreenModule.IsPreloadBackgroundImageEnable())
		{
			TArray<UTexture2D*> BackgroundImages = LoadingScreenModule.GetBackgroundImages();
			if (!BackgroundImages.IsEmpty() && BackgroundImages.IsValidIndex(ImageIndex))
			{
				LoadingImage = BackgroundImages[ImageIndex];
			}
		}

		if (LoadingImage == nullptr)
		{
			// 从设置中加载背景
			const FSoftObjectPath& ImageAsset = Settings.Images[ImageIndex];
			UObject* ImageObject = ImageAsset.TryLoad();
			LoadingImage = Cast<UTexture2D>(ImageObject);
		}

		if (LoadingImage)
		{
			ImageBrush = FDeferredCleanupSlateBrush::CreateBrush(LoadingImage);

			ChildSlot
				[
					SNew(SBorder)//带有边框的容器
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(Settings.Padding)
						.BorderBackgroundColor(Settings.BackgroundColor)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						[
							SNew(SScaleBox)//容器，在其中放置其他UI组件，并根据设置的比例对其进行缩放
								.Stretch(Settings.ImageStretch)
								[
									SNew(SImage)
										.Image(ImageBrush.IsValid() ? ImageBrush->GetSlateBrush() : nullptr)
								]
						]
				];
		}
	}

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
