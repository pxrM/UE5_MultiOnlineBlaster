// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingCompleteText.h"
#include "SlateOptMacros.h"
#include "MoviePlayer.h"
#include "LoadingScreenSettings.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoadingCompleteText::Construct(const FArguments& InArgs, const FLoadingCompleteTextSettings& CompleteTextSettings)
{
	CompleteTextColor = CompleteTextSettings.Appearance.ColorAndOpacity.GetSpecifiedColor();
	CompleteTextAnimationSpeed = CompleteTextSettings.AnimationSpeed;

	ChildSlot
		[
			SNew(STextBlock)
				.Font(CompleteTextSettings.Appearance.Font)
				.ShadowOffset(CompleteTextSettings.Appearance.ShadowOffset)
				.ShadowColorAndOpacity(CompleteTextSettings.Appearance.ShadowColorAndOpacity)
				.Justification(CompleteTextSettings.Appearance.Justification)
				.Text(CompleteTextSettings.LoadingCompleteText)
				.ColorAndOpacity(this, &SLoadingCompleteText::GetLoadingCompleteTextColor)
				.Visibility(this, &SLoadingCompleteText::GetLoadingCompleteTextVisibility)
		];

	// ע�ᶯ��ͼ�����л��ʱ���¼�
	if (CompleteTextSettings.bFadeInOutAnim && !bIsActiveTimerRegsitered)
	{
		bIsActiveTimerRegsitered = true;
		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SLoadingCompleteText::AnimateText));
	}
}

EVisibility SLoadingCompleteText::GetLoadingCompleteTextVisibility() const
{
	return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Visible : EVisibility::Hidden;
}

EActiveTimerReturnType SLoadingCompleteText::AnimateText(double InCurrentTime, float InDeltaTime)
{
	const float MinAlpha = 0.1f;
	const float MaxAlpha = 1.0f;

	float TextAlpha = CompleteTextColor.A;

	if (TextAlpha >= MaxAlpha)
	{
		bCompleteTextReverseAnim = true;
	}
	else if (TextAlpha <= MinAlpha)
	{
		bCompleteTextReverseAnim = false;
	}

	if (bCompleteTextReverseAnim)
	{
		TextAlpha += InDeltaTime * CompleteTextAnimationSpeed;
	}
	else
	{
		TextAlpha -= InDeltaTime * CompleteTextAnimationSpeed;
	}

	CompleteTextColor.A = TextAlpha;

	return EActiveTimerReturnType::Continue;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
