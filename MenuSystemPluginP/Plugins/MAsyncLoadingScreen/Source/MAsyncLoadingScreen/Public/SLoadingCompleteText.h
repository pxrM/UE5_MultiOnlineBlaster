// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FLoadingCompleteTextSettings;

/**
 *
 */
class MASYNCLOADINGSCREEN_API SLoadingCompleteText : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingCompleteText)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FLoadingCompleteTextSettings& CompleteTextSettings);

public:
	/// <summary>
	/// 获取加载完成文本的可见性
	/// </summary>
	/// <returns></returns>
	EVisibility GetLoadingCompleteTextVisibility() const;

	/// <summary>
	/// 修改动画文本的透明度，从而实现文本闪烁的效果
	/// </summary>
	/// <param name="InCurrentTime"> 当前时间 </param>
	/// <param name="InDeltaTime"> 时间增量 </param>
	/// <returns></returns>
	EActiveTimerReturnType AnimateText(double InCurrentTime, float InDeltaTime);

	/// <summary>
	/// 获取加载完成文本的颜色
	/// </summary>
	/// <returns></returns>
	FORCEINLINE FSlateColor GetLoadingCompleteTextColor() const { return CompleteTextColor; };

private:
	/// <summary>
	/// 加载成功文本颜色
	/// </summary>
	FLinearColor CompleteTextColor = FLinearColor::White;

	/// <summary>
	/// 是否需要反转动画
	/// </summary>
	bool bCompleteTextReverseAnim = false;

	/// <summary>
	/// 文本动画速度
	/// </summary>
	float CompleteTextAnimationSpeed = 1.0f;

	/// <summary>
	/// 是否注册了计时器
	/// </summary>
	bool bIsActiveTimerRegsitered = false;
};
