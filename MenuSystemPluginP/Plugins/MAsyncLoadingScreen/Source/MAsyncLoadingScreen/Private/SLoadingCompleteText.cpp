// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingCompleteText.h"
#include "SlateOptMacros.h"
#include "MoviePlayer.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoadingCompleteText::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
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
