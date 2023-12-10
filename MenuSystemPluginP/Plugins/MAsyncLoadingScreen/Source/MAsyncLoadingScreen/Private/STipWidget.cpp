// Fill out your copyright notice in the Description page of Project Settings.


#include "STipWidget.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenFuncLibrary.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STipWidget::Construct(const FArguments& InArgs, const FTipSettings& Settings)
{
	if (Settings.TipTexts.Num() > 0)
	{
		int32 TipIndex = FMath::RandRange(0, Settings.TipTexts.Num() - 1);

		if (Settings.bSetDisplayTipTextManually)
		{
			if (Settings.TipTexts.IsValidIndex(UAsyncLoadingScreenFuncLibrary::GetDisplayTipTextIndex()))
			{
				TipIndex = UAsyncLoadingScreenFuncLibrary::GetDisplayTipTextIndex();
			}
		}

		// Slate�У�ChildSlot��һ�����ڶ����Ӳ������ֺ����Ե�������
		// [ ] �е����ݱ�ʾ ChildSlot ������һ����������������Է����Ӳ������������ǵĲ��ֺ����ԡ�
		ChildSlot
			[
				SNew(STextBlock)
					.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
					.Font(Settings.Appearance.Font)
					.ShadowOffset(Settings.Appearance.ShadowOffset)
					.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
					.Justification(Settings.Appearance.Justification)
					.WrapTextAt(Settings.TipWrapAt)
					.Text(Settings.TipTexts[TipIndex])
			];

	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
